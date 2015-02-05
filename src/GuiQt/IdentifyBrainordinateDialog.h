#ifndef __IDENTIFY_BRAINORDINATE_DIALOG_H__
#define __IDENTIFY_BRAINORDINATE_DIALOG_H__

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

#include "EventListenerInterface.h"
#include "WuQDialogNonModal.h"

class QLabel;
class QRadioButton;
class QSpinBox;

namespace caret {

    class CaretDataFileSelectionComboBox;
    class CaretDataFileSelectionModel;
    class CaretMappableDataFileAndMapSelectorObject;
    class CaretMappableDataFile;
    class CiftiParcelSelectionComboBox;
    class StructureEnumComboBox;
    
    class IdentifyBrainordinateDialog : public WuQDialogNonModal, public EventListenerInterface {
        
        Q_OBJECT

    public:
        IdentifyBrainordinateDialog(QWidget* parent);
        
        virtual ~IdentifyBrainordinateDialog();
        
        virtual void updateDialog();
        
        virtual void receiveEvent(Event* event);
        
    protected:
        virtual void applyButtonClicked();
        
    private:
        IdentifyBrainordinateDialog(const IdentifyBrainordinateDialog&);

        IdentifyBrainordinateDialog& operator=(const IdentifyBrainordinateDialog&);
        
    public:

        // ADD_NEW_METHODS_HERE

    private slots:
        void slotParcelFileOrMapSelectionChanged();
        
    private:
        enum Mode {
            MODE_NONE,
            MODE_CIFTI_PARCEL,
            MODE_CIFTI_ROW,
            MODE_SURFACE_VERTEX
        };
        
        // ADD_NEW_MEMBERS_HERE
        
        QRadioButton* m_vertexRadioButton;
        
        StructureEnumComboBox* m_vertexStructureComboBox;
        
        QLabel* m_vertexStructureLabel;
        
        QSpinBox* m_vertexIndexSpinBox;
        
        QLabel* m_vertexIndexLabel;
        
        QRadioButton* m_ciftiRowFileRadioButton;
        
        QLabel* m_ciftiRowFileLabel;
        
        CaretDataFileSelectionComboBox* m_ciftiRowFileComboBox;

        CaretDataFileSelectionModel* m_ciftiRowFileSelectionModel;
        
        QLabel* m_ciftiRowFileIndexLabel;
        
        QSpinBox* m_ciftiRowFileIndexSpinBox;
        
        QRadioButton* m_ciftiParcelFileRadioButton;
        
        QLabel* m_ciftiParcelFileLabel;
        
        QLabel* m_ciftiParcelFileMapLabel;
        
        QWidget* m_ciftiParcelFileComboBox;
        
        QWidget* m_ciftiParcelFileMapSpinBox;
        
        QWidget* m_ciftiParcelFileMapComboBox;
        
        QLabel* m_ciftiParcelFileParcelLabel;
        
        CiftiParcelSelectionComboBox* m_ciftiParcelFileParcelNameComboBox;
        
        CaretMappableDataFileAndMapSelectorObject* m_ciftiParcelFileSelector;
    };
    
#ifdef __IDENTIFY_BRAINORDINATE_DIALOG_DECLARE__
#endif // __IDENTIFY_BRAINORDINATE_DIALOG_DECLARE__

} // namespace
#endif  //__IDENTIFY_BRAINORDINATE_DIALOG_H__
