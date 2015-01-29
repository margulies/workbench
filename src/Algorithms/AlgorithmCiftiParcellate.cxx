/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#include "AlgorithmCiftiParcellate.h"
#include "AlgorithmException.h"
#include "CiftiFile.h"
#include "GiftiLabel.h"
#include "GiftiLabelTable.h"
#include "MultiDimIterator.h"
#include "ReductionOperation.h"

#include <cmath>
#include <map>

using namespace caret;
using namespace std;

AString AlgorithmCiftiParcellate::getCommandSwitch()
{
    return "-cifti-parcellate";
}

AString AlgorithmCiftiParcellate::getShortDescription()
{
    return "PARCELLATE A CIFTI FILE";
}

OperationParameters* AlgorithmCiftiParcellate::getParameters()
{
    OperationParameters* ret = new OperationParameters();
    ret->addCiftiParameter(1, "cifti-in", "the cifti file to parcellate");
    
    ret->addCiftiParameter(2, "cifti-label", "a cifti label file to use for the parcellation");
    
    ret->addStringParameter(3, "direction", "which mapping to parcellate, ROW or COLUMN");
    
    ret->addCiftiOutputParameter(4, "cifti-out", "output cifti file");
    
    ret->setHelpText(
        AString("Each label in the cifti label file will be treated as a parcel, and all rows or columns within the parcel are averaged together to form the output ") +
        "row or column.  " +
        "If ROW is specified, then the input mapping along rows must be brainordinates, and the output mapping along rows will be parcels, meaning columns will be averaged together.  " +
        "For dtseries or dscalar, use COLUMN."
    );
    return ret;
}

void AlgorithmCiftiParcellate::useParameters(OperationParameters* myParams, ProgressObject* myProgObj)
{
    CiftiFile* myCiftiIn = myParams->getCifti(1);
    CiftiFile* myCiftiLabel = myParams->getCifti(2);
    AString dirString = myParams->getString(3);
    int direction = -1;
    if (dirString == "ROW")
    {
        direction = CiftiXML::ALONG_ROW;
    } else {
        if (dirString == "COLUMN")
        {
            direction = CiftiXML::ALONG_COLUMN;
        } else {
            throw AlgorithmException("unrecognized direction string, use ROW or COLUMN");
        }
    }
    CiftiFile* myCiftiOut = myParams->getOutputCifti(4);
    AlgorithmCiftiParcellate(myProgObj, myCiftiIn, myCiftiLabel, direction, myCiftiOut);
}

AlgorithmCiftiParcellate::AlgorithmCiftiParcellate(ProgressObject* myProgObj, const CiftiFile* myCiftiIn, const CiftiFile* myCiftiLabel, const int& direction, CiftiFile* myCiftiOut) : AbstractAlgorithm(myProgObj)
{
    LevelProgress myProgress(myProgObj);
    const CiftiXML& myInputXML = myCiftiIn->getCiftiXML();
    const CiftiXML& myLabelXML = myCiftiLabel->getCiftiXML();
    vector<int64_t> dims = myInputXML.getDimensions();
    if (direction >= (int)dims.size()) throw AlgorithmException("specified direction doesn't exist in input file");
    if (myInputXML.getMappingType(direction) != CiftiMappingType::BRAIN_MODELS)
    {
        throw AlgorithmException("input cifti file does not have brain models mapping type in specified direction");
    }
    if (myLabelXML.getNumberOfDimensions() != 2 ||
        myLabelXML.getMappingType(CiftiXML::ALONG_ROW) != CiftiMappingType::LABELS ||
        myLabelXML.getMappingType(CiftiXML::ALONG_COLUMN) != CiftiMappingType::BRAIN_MODELS)
    {
        throw AlgorithmException("input cifti label file has the wrong mapping types");
    }
    const CiftiBrainModelsMap& inputDense = myInputXML.getBrainModelsMap(direction);
    const CiftiBrainModelsMap& labelDense = myLabelXML.getBrainModelsMap(CiftiXML::ALONG_COLUMN);
    if (inputDense.hasVolumeData())
    {//don't check volume space if direction doesn't have volume data
        if (labelDense.hasVolumeData() && !inputDense.getVolumeSpace().matches(labelDense.getVolumeSpace()))
        {
            throw AlgorithmException("input cifti files must have the same volume space");
        }
    }
    vector<int> indexToParcel;
    CiftiXML myOutXML = myInputXML;
    CiftiParcelsMap outParcelMap = parcellateMapping(myCiftiLabel, inputDense, indexToParcel);
    int numParcels = outParcelMap.getLength();
    if (numParcels < 1)
    {
        throw AlgorithmException("no parcels found, output file would be empty, aborting");
    }
    myOutXML.setMap(direction, outParcelMap);
    myCiftiOut->setCiftiXML(myOutXML);
    int64_t numCols = myInputXML.getDimensionLength(CiftiXML::ALONG_ROW);
    vector<float> scratchRow(numCols);
    vector<int64_t> parcelCounts(numParcels, 0);
    for (int64_t j = 0; j < (int64_t)indexToParcel.size(); ++j)
    {
        int parcel = indexToParcel[j];
        CaretAssert(parcel > -2 && parcel < numParcels);
        if (parcel != -1)
        {
            ++parcelCounts[parcel];
        }
    }
    bool isLabel = false;
    int labelDir = -1;
    for (int i = 0; i < (int)dims.size(); ++i)
    {
        if (myInputXML.getMappingType(i) == CiftiMappingType::LABELS)
        {
            isLabel = true;
            labelDir = i;
            break;//there should never be more than one dimension with LABEL type, and if there is, just use the first one, i guess...
        }
    }
    if (direction == CiftiXML::ALONG_ROW)
    {
        vector<float> scratchOutRow(numParcels);
        if (isLabel)
        {
            vector<vector<float> > parcelData(numParcels);//float so we can use ReductionOperation (when not considering vertex area, etc)
            for (int j = 0; j < numParcels; ++j)
            {
                parcelData[j].reserve(parcelCounts[j]);
            }
            for (MultiDimIterator<int64_t> iter(vector<int64_t>(dims.begin() + 1, dims.end())); !iter.atEnd(); ++iter)
            {
                for (int j = 0; j < numParcels; ++j)
                {
                    parcelData[j].clear();//doesn't change allocation
                }
                myCiftiIn->getRow(scratchRow.data(), *iter);
                for (int64_t j = 0; j < numCols; ++j)
                {
                    int parcel = indexToParcel[j];
                    if (parcel != -1)
                    {
                        parcelData[parcel].push_back(floor(scratchRow[j] + 0.5f));//round to nearest integer to be safe
                    }
                }
                for (int j = 0; j < numParcels; ++j)
                {
                    CaretAssert(parcelCounts[j] == (int64_t)parcelData[j].size());
                    if (parcelCounts[j] > 0)
                    {
                        scratchOutRow[j] = ReductionOperation::reduce(parcelData[j].data(), parcelData[j].size(), ReductionEnum::MODE);
                    } else {//labelDir can't be 0 (row) because we are parcellating along row, so row must be dense
                        scratchOutRow[j] = myOutXML.getLabelsMap(labelDir).getMapLabelTable((*iter)[labelDir - 1])->getUnassignedLabelKey();
                    }
                }
                myCiftiOut->setRow(scratchOutRow.data(), *iter);
            }
        } else {
            for (MultiDimIterator<int64_t> iter(vector<int64_t>(dims.begin() + 1, dims.end())); !iter.atEnd(); ++iter)
            {
                vector<double> scratchAccum(numParcels, 0.0);
                myCiftiIn->getRow(scratchRow.data(), *iter);
                for (int64_t j = 0; j < numCols; ++j)
                {
                    int parcel = indexToParcel[j];
                    if (parcel != -1)
                    {
                        scratchAccum[parcel] += scratchRow[j];
                    }
                }
                for (int j = 0; j < numParcels; ++j)
                {
                    if (parcelCounts[j] > 0)
                    {
                        scratchOutRow[j] = scratchAccum[j] / parcelCounts[j];
                    } else {
                        scratchOutRow[j] = 0.0f;
                    }
                }
                myCiftiOut->setRow(scratchOutRow.data(), *iter);
            }
        }
    } else {
        vector<float> scratchOutRow(numCols);
        vector<int64_t> otherDims = dims;
        otherDims.erase(otherDims.begin() + direction);//direction being parcellated
        otherDims.erase(otherDims.begin());//row
        if (isLabel)
        {
            vector<vector<vector<float> > > parcelData(numParcels, vector<vector<float> >(numCols));//float so we can use ReductionOperation (when not considering vertex area, etc)
            for (int i = 0; i < numParcels; ++i)
            {
                for (int j = 0; j < numCols; ++j)
                {
                    parcelData[i][j].reserve(parcelCounts[i]);
                }
            }
            for (MultiDimIterator<int64_t> iter(otherDims); !iter.atEnd(); ++iter)
            {
                vector<int64_t> indices(dims.size() - 1);//we need to add the parcellated direction index back into the index list to use it in getRow/setRow
                for (int i = 0; i < (int)otherDims.size(); ++i)
                {
                    if (i < direction - 1)
                    {
                        indices[i] = (*iter)[i];
                    } else {
                        indices[i + 1] = (*iter)[i];
                    }
                }//indices[direction - 1] is uninitialized, as it is the dimension to be parcellated
                for (int i = 0; i < numParcels; ++i)
                {
                    for (int j = 0; j < numCols; ++j)
                    {
                        parcelData[i][j].clear();//doesn't change allocation
                    }
                }
                for (int64_t i = 0; i < dims[direction]; ++i)
                {
                    int parcel = indexToParcel[i];
                    if (parcel != -1)
                    {
                        indices[direction - 1] = i;
                        myCiftiIn->getRow(scratchRow.data(), indices);
                        vector<vector<float> >& parcelRef = parcelData[parcel];
                        for (int j = 0; j < numCols; ++j)
                        {
                            parcelRef[j].push_back(floor(scratchRow[j] + 0.5f));
                        }
                    }
                }
                for (int i = 0; i < numParcels; ++i)
                {
                    indices[direction - 1] = i;
                    int64_t count = parcelCounts[i];
                    vector<vector<float> >& parcelRef = parcelData[i];
                    if (count > 0)
                    {
                        for (int j = 0; j < numCols; ++j)
                        {
                            CaretAssert((int64_t)parcelRef[j].size() == count);
                            scratchOutRow[j] = ReductionOperation::reduce(parcelRef[j].data(), parcelRef[j].size(), ReductionEnum::MODE);
                        }
                    } else {
                        for (int j = 0; j < numCols; ++j)
                        {
                            CaretAssert((int64_t)parcelRef[j].size() == count);
                            if (labelDir == CiftiXML::ALONG_ROW)
                            {
                                scratchOutRow[j] = myOutXML.getLabelsMap(CiftiXML::ALONG_ROW).getMapLabelTable(j)->getUnassignedLabelKey();
                            } else {
                                scratchOutRow[j] = myOutXML.getLabelsMap(labelDir).getMapLabelTable(indices[labelDir - 1])->getUnassignedLabelKey();
                            }
                        }
                    }
                    myCiftiOut->setRow(scratchOutRow.data(), indices);
                }
            }
        } else {
            for (MultiDimIterator<int64_t> iter(otherDims); !iter.atEnd(); ++iter)
            {
                vector<int64_t> indices(dims.size() - 1);//we need to add the parcellated direction index back into the index list to use it in getRow/setRow
                for (int i = 0; i < (int)otherDims.size(); ++i)
                {
                    if (i < direction - 1)
                    {
                        indices[i] = (*iter)[i];
                    } else {
                        indices[i + 1] = (*iter)[i];
                    }
                }//indices[direction - 1] is uninitialized, as it is the dimension to be parcellated
                vector<vector<double> > accumRows(numParcels, vector<double>(numCols, 0.0f));
                for (int64_t i = 0; i < dims[direction]; ++i)
                {
                    int parcel = indexToParcel[i];
                    if (parcel != -1)
                    {
                        indices[direction - 1] = i;
                        myCiftiIn->getRow(scratchRow.data(), indices);
                        vector<double>& parcelRowRef = accumRows[parcel];
                        for (int64_t j = 0; j < numCols; ++j)
                        {
                            parcelRowRef[j] += scratchRow[j];
                        }
                    }
                }
                for (int i = 0; i < numParcels; ++i)
                {
                    indices[direction - 1] = i;
                    int64_t count = parcelCounts[i];
                    if (count > 0)
                    {
                        vector<double>& parcelRowRef = accumRows[i];
                        for (int64_t j = 0; j < numCols; ++j)
                        {
                            scratchOutRow[j] = parcelRowRef[j] / count;
                        }
                    } else {
                        for (int64_t j = 0; j < numCols; ++j)
                        {
                            scratchOutRow[j] = 0.0f;
                        }
                    }
                    myCiftiOut->setRow(scratchOutRow.data(), indices);
                }
            }
        }
    }
}

CiftiParcelsMap AlgorithmCiftiParcellate::parcellateMapping(const CiftiFile* myCiftiLabel, const CiftiBrainModelsMap& toParcellate, vector<int>& indexToParcelOut)
{
    const CiftiXML& myLabelXML = myCiftiLabel->getCiftiXML();
    if (myLabelXML.getNumberOfDimensions() != 2 ||
        myLabelXML.getMappingType(CiftiXML::ALONG_ROW) != CiftiMappingType::LABELS ||
        myLabelXML.getMappingType(CiftiXML::ALONG_COLUMN) != CiftiMappingType::BRAIN_MODELS)
    {
        throw AlgorithmException("AlgorithmCiftiParcellate::parcellateMapping requires a cifti dlabel file as input");
    }
    const CiftiLabelsMap& myLabelsMap = myLabelXML.getLabelsMap(CiftiXML::ALONG_ROW);
    const CiftiBrainModelsMap& myDenseMap = myLabelXML.getBrainModelsMap(CiftiXML::ALONG_COLUMN);
    CiftiParcelsMap ret;
    if (toParcellate.hasVolumeData() && myDenseMap.hasVolumeData())
    {
        if(!toParcellate.getVolumeSpace().matches(myDenseMap.getVolumeSpace()))
        {
            throw AlgorithmException("AlgorithmCiftiParcellate::parcellateMapping requires matching volume space between dlabel and dense mapping to parcellate");
        }
        ret.setVolumeSpace(toParcellate.getVolumeSpace());
    }
    const GiftiLabelTable* myLabelTable = myLabelsMap.getMapLabelTable(0);
    vector<float> labelData(myLabelXML.getDimensionLength(CiftiXML::ALONG_COLUMN));
    int unusedKey = myLabelTable->getUnassignedLabelKey();
    myCiftiLabel->getColumn(labelData.data(), 0);
    map<int, pair<CiftiParcelsMap::Parcel, int> > usedKeys;//the keys from the label table that actually overlap with data in the input file
    indexToParcelOut.clear();
    indexToParcelOut.resize(toParcellate.getLength(), -1);
    vector<StructureEnum::Enum> surfList = toParcellate.getSurfaceStructureList();
    vector<StructureEnum::Enum> volList = toParcellate.getVolumeStructureList();
    for (int i = 0; i < (int)surfList.size(); ++i)
    {
        StructureEnum::Enum myStruct = surfList[i];
        if (myDenseMap.hasSurfaceData(myStruct) && toParcellate.hasSurfaceData(myStruct))
        {
            if (myDenseMap.getSurfaceNumberOfNodes(myStruct) != toParcellate.getSurfaceNumberOfNodes(myStruct))
            {
                throw AlgorithmException("mismatch in number of surface vertices between input and dlabel for structure " + StructureEnum::toName(myStruct));
            }
            ret.addSurface(toParcellate.getSurfaceNumberOfNodes(myStruct), myStruct);
            vector<CiftiBrainModelsMap::SurfaceMap> surfMap = toParcellate.getSurfaceMap(myStruct);
            int64_t mapSize = (int64_t)surfMap.size();
            for (int64_t j = 0; j < mapSize; ++j)
            {
                int64_t labelIndex = myDenseMap.getIndexForNode(surfMap[j].m_surfaceNode, myStruct);
                if (labelIndex != -1)
                {
                    int labelKey = (int)floor(labelData[labelIndex] + 0.5f);
                    if (labelKey != unusedKey)
                    {
                        int tempVal = -1;
                        map<int, pair<CiftiParcelsMap::Parcel, int> >::iterator iter = usedKeys.find(labelKey);
                        if (iter == usedKeys.end())
                        {
                            const GiftiLabel* myLabel = myLabelTable->getLabel(labelKey);
                            if (myLabel != NULL)//ignore values that aren't in the label table
                            {
                                tempVal = usedKeys.size();
                                CiftiParcelsMap::Parcel tempParcel;
                                tempParcel.m_name = myLabel->getName();
                                tempParcel.m_surfaceNodes[myStruct].insert(surfMap[j].m_surfaceNode);
                                usedKeys[labelKey] = pair<CiftiParcelsMap::Parcel, int>(tempParcel, tempVal);
                            }
                        } else {
                            tempVal = iter->second.second;
                            CiftiParcelsMap::Parcel& tempParcel = iter->second.first;
                            tempParcel.m_surfaceNodes[myStruct].insert(surfMap[j].m_surfaceNode);
                        }
                        indexToParcelOut[surfMap[j].m_ciftiIndex] = tempVal;//we will remap these to be in order of label keys later
                    }
                }
            }
        }
    }
    vector<CiftiBrainModelsMap::VolumeMap> volMap = toParcellate.getFullVolumeMap();
    int64_t mapSize = (int64_t)volMap.size();
    for (int64_t i = 0; i < mapSize; ++i)
    {
        int64_t labelIndex = myDenseMap.getIndexForVoxel(volMap[i].m_ijk);
        if (labelIndex != -1)
        {
            int labelKey = (int)floor(labelData[labelIndex] + 0.5f);
            if (labelKey != unusedKey)
            {
                int tempVal = -1;
                map<int, pair<CiftiParcelsMap::Parcel, int> >::iterator iter = usedKeys.find(labelKey);
                if (iter == usedKeys.end())
                {
                    const GiftiLabel* myLabel = myLabelTable->getLabel(labelKey);
                    if (myLabel != NULL)//ignore values that aren't in the label table
                    {
                        tempVal = usedKeys.size();
                        CiftiParcelsMap::Parcel tempParcel;
                        tempParcel.m_name = myLabel->getName();
                        tempParcel.m_voxelIndices.insert(VoxelIJK(volMap[i].m_ijk));
                        usedKeys[labelKey] = pair<CiftiParcelsMap::Parcel, int>(tempParcel, tempVal);
                    }
                } else {
                    tempVal = iter->second.second;
                    CiftiParcelsMap::Parcel& tempParcel = iter->second.first;
                    tempParcel.m_voxelIndices.insert(VoxelIJK(volMap[i].m_ijk));
                }
                indexToParcelOut[volMap[i].m_ciftiIndex] = tempVal;//we will remap these to be in order of label keys later
            }
        }
    }
    int numParcels = (int)usedKeys.size();
    vector<int> valRemap(numParcels, -1);
    int count = 0;
    for (map<int, pair<CiftiParcelsMap::Parcel, int> >::const_iterator iter = usedKeys.begin(); iter != usedKeys.end(); ++iter)
    {
        valRemap[iter->second.second] = count;//build a lookup from temp values to label key rank
        ret.addParcel(iter->second.first);
        ++count;
    }
    int64_t lookupSize = (int64_t)indexToParcelOut.size();
    for (int64_t i = 0; i < lookupSize; ++i)//finally, remap the temporary values to the key order of the labels
    {
        if (indexToParcelOut[i] != -1)
        {
            indexToParcelOut[i] = valRemap[indexToParcelOut[i]];
        }
    }
    return ret;
}

float AlgorithmCiftiParcellate::getAlgorithmInternalWeight()
{
    return 1.0f;//override this if needed, if the progress bar isn't smooth
}

float AlgorithmCiftiParcellate::getSubAlgorithmWeight()
{
    //return AlgorithmInsertNameHere::getAlgorithmWeight();//if you use a subalgorithm
    return 0.0f;
}
