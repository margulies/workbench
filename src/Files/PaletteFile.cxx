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

#include "CaretLogger.h"
#include "DataFileException.h"
#include "GiftiLabel.h"
#include "GiftiMetaData.h"
#include "Palette.h"
#include "PaletteColorMapping.h"
#include "PaletteFile.h"
#include "PaletteScalarAndColor.h"

#include <limits>

using namespace caret;

/**
 * Constructor.
 *
 */
PaletteFile::PaletteFile()
: CaretDataFile(DataFileTypeEnum::PALETTE)
{
    this->metadata = new GiftiMetaData();
    this->initializeMembersPaletteFile();
    this->addDefaultPalettes();
    this->clearModified();
}

/**
 * Destructor
 */
PaletteFile::~PaletteFile()
{
    this->clearAll();
    delete this->metadata;
}

void
PaletteFile::initializeMembersPaletteFile()
{
}

/**
 * Get the label table used for color storage.
 * @return  LabelTable used for color storage.
 *
 */
GiftiLabelTable*
PaletteFile::getLabelTable()
{
    return &this->labelTable;
}

/**
 * Clear everything.
 */
void
PaletteFile::clearAll()
{
    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        delete this->palettes[i];
    }
    this->palettes.clear();
    this->labelTable.clear();
    this->metadata->clear();
}

/**
 * Clear the file but add default palettes.
 */
void
PaletteFile::clear()
{
    this->clearAll();
    this->addDefaultPalettes();
}

/**
 * Add a palette color.
 *
 * @param pc - color to add.
 *
 */
void
PaletteFile::addColor(const GiftiLabel& pc)
{
    this->labelTable.addLabel(&pc);
}

/**
 * Add a palette color.
 *
 * @param name  - name of color.
 * @param red   - red component.
 * @param green - red component.
 * @param blue  - red component.
 *
 */
void
PaletteFile::addColor(
                   const AString& name,
                   const int32_t red,
                   const int32_t green,
                   const int32_t blue)
{
    this->labelTable.addLabel(name, red, green, blue);
}

/**
 * Add a palette color.
 *
 * @param name - Name of color.
 * @param rgb  - RGB components of color.
 *
 */
void
PaletteFile::addColor(
                   const AString& name,
                   const int32_t rgb[])
{
    this->addColor(name, rgb[0], rgb[1], rgb[2]);
}

/**
 * Get a color via its index.
 *
 * @param index - index of color.
 * @return  Reference to color at index or the default color
 *          if the index is invalid.
 *
 */
const GiftiLabel*
PaletteFile::getColor(const int32_t indx) const
{
    return this->labelTable.getLabel(indx);
}

/**
 * Get a color via its index.
 *
 * @param colorName - Name of color.
 * @return  Reference to color with name or the default color
 *          if the name does not match any colors.
 *
 */
const GiftiLabel*
PaletteFile::getColorByName(const AString& colorName) const
{
    const GiftiLabel* gl = this->labelTable.getLabel(colorName);
    return gl;
}

/**
 * Get index for a color.
 *
 * @param colorName - Name of color.
 * @return  Index to color or -1 if not found.
 *
 */
int32_t
PaletteFile::getColorIndex(const AString& colorName) const
{
    return this->labelTable.getLabelKeyFromName(colorName);
}

/**
 * Get the number of palettes.
 *
 * @return The number of palettes.
 *
 */
int32_t
PaletteFile::getNumberOfPalettes() const
{
    return this->palettes.size();
}

/**
 * Add a palette.
 *
 * @param p - palette to add.
 *
 */
void
PaletteFile::addPalette(const Palette& p)
{
    Palette* pal = new Palette(p);
    this->assignColorsToPalette(*pal);
    this->palettes.push_back(pal);
    this->setModified();
}

/**
 * Get a palette.
 *
 * @param index - index of palette.
 * @return  Reference to palette or null if invalid index.
 *
 */
Palette*
PaletteFile::getPalette(const int32_t indx) const
{
    return this->palettes[indx];
}

/**
 * Find a palette by the specified name.
 *
 * @param name  Name of palette to search for.
 * @return  Reference to palette with name or null if not found.
 *
 */
Palette*
PaletteFile::getPaletteByName(const AString& name) const
{
    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        if (this->palettes[i]->getName() == name) {
            return this->palettes[i];
        }
    }
    return NULL;
}

/**
 * Remove a palette.
 *
 * @param index - index of palette to remove.
 *
 */
void
PaletteFile::removePalette(const int32_t indx)
{
    this->palettes.erase(this->palettes.begin() + indx);
    this->setModified();
}

/**
 * Is this file empty?
 *
 * @return true if the file is empty, else false.
 *
 */
bool
PaletteFile::isEmpty() const
{
    return this->palettes.empty();
}

/**
 * String description of this class.
 */
AString
PaletteFile::toString() const
{
    AString s;

    int64_t numberOfPalettes = this->palettes.size();
    for (int64_t i = 0; i < numberOfPalettes; i++) {
        s += (this->palettes[i]->toString() + "\n");
    }

    return s;
}

/**
 * Is this palette modified?
 * @return
 *   true if modified, else false.
 */
bool
PaletteFile::isModified() const
{
    if (DataFile::isModified()) {
        return true;
    }
    if (this->labelTable.isModified()) {
        return true;
    }

    const int64_t numberOfPalettes = this->getNumberOfPalettes();
    for (int i = 0; i < numberOfPalettes; i++) {
        if (this->palettes[i]->isModified()) {
            return true;
        }
    }
    return false;
}

/**
 * Set this object as not modified.  Object should also
 * clear the modification status of its children.
 *
 */
void
PaletteFile::clearModified()
{
    DataFile::clearModified();

    const int64_t numberOfPalettes = this->getNumberOfPalettes();
    for (int i = 0; i < numberOfPalettes; i++) {
        this->palettes[i]->clearModified();
    }

    this->labelTable.clearModified();
}

/**
 * Assign colors to the palette.
 * @param
 *    p Palette to which colors are assigned.
 */
void
PaletteFile::assignColorsToPalette(Palette& p)
{
    int64_t numberOfScalars = p.getNumberOfScalarsAndColors();
    for (int64_t i = 0; i < numberOfScalars; i++) {
        PaletteScalarAndColor* psac = p.getScalarAndColor(i);
        const AString& colorName = psac->getColorName();
        const GiftiLabel* gl = this->getColorByName(colorName);
        if (gl != NULL) {
            float rgba[4];
            gl->getColor(rgba);
            psac->setColor(rgba);
        } else {
            CaretLogSevere(("Missing color \""
                            + colorName
                            + "\" in palette \""
                            + p.getName()
                            + "\""));
        }
    }
}

/**
 * Read the data file.
 *
 * @param filename
 *    Name of the data file.
 * @throws DataFileException
 *    If the file was not successfully read.
 */
void
PaletteFile::readFile(const AString& filename)
{
    clear();
//    checkFileReadability(filename);

    throw DataFileException(filename,
                            "Reading of PaletteFile not implemented.");
}

/**
 * Write the data file.
 *
 * @param filename
 *    Name of the data file.
 * @throws DataFileException
 *    If the file was not successfully written.
 */
void
PaletteFile::writeFile(const AString& filename)
{
//    checkFileWritability(filename);

    throw DataFileException(filename,
                            "Reading of PaletteFile not implemented.");
}

/**
 * Add the default palettes.
 *
 */
void
PaletteFile::addDefaultPalettes()
{
    bool modifiedStatus = this->isModified();

    this->addColor("none",  0xff, 0xff, 0xff );
    this->addColor("_yellow",  0xff, 0xff, 0x00 );
    this->addColor("_black",  0x00, 0x00, 0x00 );
    this->addColor("_orange",  0xff, 0x69, 0x00 );

    //----------------------------------------------------------------------
    // Psych palette
    //
    if (this->getPaletteByName("PSYCH") == NULL) {
        this->addColor("_pyell-oran",  0xff, 0xcc, 0x00 );
        this->addColor("_poran-red",  0xff, 0x44, 0x00 );
        this->addColor("_pblue",  0x00, 0x44, 0xff );
        this->addColor("_pltblue1",  0x00, 0x69, 0xff );
        this->addColor("_pltblue2",  0x00, 0x99, 0xff );
        this->addColor("_pbluecyan",  0x00, 0xcc, 0xff );

        Palette psych;
        psych.setName("PSYCH");
        //psych.setPositiveOnly(false);

        psych.addScalarAndColor(1.00f, "_yellow");
        psych.addScalarAndColor(0.75f, "_pyell-oran");
        psych.addScalarAndColor(0.50f, "_orange");
        psych.addScalarAndColor(0.25f, "_poran-red");
        psych.addScalarAndColor(0.05f, "none");
        psych.addScalarAndColor(-0.05f, "_pblue");
        psych.addScalarAndColor(-0.25f, "_pltblue1");
        psych.addScalarAndColor(-0.50f, "_pltblue2");
        psych.addScalarAndColor(-0.75f, "_pbluecyan");

        addPalette(psych);
    }
    //----------------------------------------------------------------------
    // Psych no-none palette
    //
    if (this->getPaletteByName("PSYCH-NO-NONE") == NULL) {
        this->addColor("_pyell-oran",  0xff, 0xcc, 0x00 );
        this->addColor("_poran-red",  0xff, 0x44, 0x00 );
        this->addColor("_pblue",  0x00, 0x44, 0xff );
        this->addColor("_pltblue1",  0x00, 0x69, 0xff );
        this->addColor("_pltblue2",  0x00, 0x99, 0xff );
        this->addColor("_pbluecyan",  0x00, 0xcc, 0xff );

        Palette psychNoNone;
        psychNoNone.setName("PSYCH-NO-NONE");
        //psychNoNone.setPositiveOnly(false);

        psychNoNone.addScalarAndColor(1.00f, "_yellow");
        psychNoNone.addScalarAndColor(0.75f, "_pyell-oran");
        psychNoNone.addScalarAndColor(0.50f, "_orange");
        psychNoNone.addScalarAndColor(0.25f, "_poran-red");
        psychNoNone.addScalarAndColor(0.0f, "_pblue");
        psychNoNone.addScalarAndColor(-0.25f, "_pltblue1");
        psychNoNone.addScalarAndColor(-0.50f, "_pltblue2");
        psychNoNone.addScalarAndColor(-0.75f, "_pbluecyan");

        addPalette(psychNoNone);
    }

    //----------------------------------------------------------------------
    // ROY-BIG palette
    //
    if (this->getPaletteByName("ROY-BIG") == NULL) {
        this->addColor("_RGB_255_255_0",  255, 255, 0 ); //#ffff00
        this->addColor("_RGB_255_200_0",  255, 200, 0 ); //#ffc800
        this->addColor("_RGB_255_120_0",  255, 120, 0 ); //#ff7800
        this->addColor("_RGB_255_0_0",  255, 0, 0 ); //#ff0000
        this->addColor("_RGB_200_0_0",  200, 0, 0 ); //#c80000
        this->addColor("_RGB_150_0_0",  150, 0, 0 ); //#960000
        this->addColor("_RGB_100_0_0",  100, 0, 0 ); //#640000
        this->addColor("_RGB_60_0_0",  60, 0, 0 ); //#3c0000
        this->addColor("_RGB_0_0_80",  0, 0, 80 ); //#000050
        this->addColor("_RGB_0_0_170",  0, 0, 170 ); //#0000aa
        this->addColor("_RGB_75_0_125",  75, 0, 125 ); //#4b007d
        this->addColor("_RGB_125_0_160",  125, 0, 160 ); //#7d00a0
        this->addColor("_RGB_75_125_0",  75, 125, 0 ); //#4b7d00
        this->addColor("_RGB_0_200_0",  0, 200, 0 ); //#00c800
        this->addColor("_RGB_0_255_0",  0, 255, 0 ); //#00ff00
        this->addColor("_RGB_0_255_255",  0, 255, 255 ); //#00ffff

        Palette royBig;
        royBig.setName("ROY-BIG");

        royBig.addScalarAndColor(1.00f, "_RGB_255_255_0");
        royBig.addScalarAndColor(0.875f, "_RGB_255_200_0");
        royBig.addScalarAndColor(0.750f, "_RGB_255_120_0");
        royBig.addScalarAndColor(0.625f, "_RGB_255_0_0");
        royBig.addScalarAndColor(0.500f, "_RGB_200_0_0");
        royBig.addScalarAndColor(0.375f, "_RGB_150_0_0");
        royBig.addScalarAndColor(0.250f, "_RGB_100_0_0");
        royBig.addScalarAndColor(0.125f, "_RGB_60_0_0");
        royBig.addScalarAndColor(0.000f, "none");
        royBig.addScalarAndColor(-0.125f, "_RGB_0_0_80");
        royBig.addScalarAndColor(-0.250f, "_RGB_0_0_170");
        royBig.addScalarAndColor(-0.375f, "_RGB_75_0_125");
        royBig.addScalarAndColor(-0.500f, "_RGB_125_0_160");
        royBig.addScalarAndColor(-0.625f, "_RGB_75_125_0");
        royBig.addScalarAndColor(-0.750f, "_RGB_0_200_0");
        royBig.addScalarAndColor(-0.875f, "_RGB_0_255_0");
        royBig.addScalarAndColor(-0.990f, "_RGB_0_255_255");
        royBig.addScalarAndColor(-1.00f, "_RGB_0_255_255");

        addPalette(royBig);


        Palette royBigBL;
        royBigBL.setName(Palette::ROY_BIG_BL_PALETTE_NAME);

        royBigBL.addScalarAndColor(1.00f, "_RGB_255_255_0");
        royBigBL.addScalarAndColor(0.875f, "_RGB_255_200_0");
        royBigBL.addScalarAndColor(0.750f, "_RGB_255_120_0");
        royBigBL.addScalarAndColor(0.625f, "_RGB_255_0_0");
        royBigBL.addScalarAndColor(0.500f, "_RGB_200_0_0");
        royBigBL.addScalarAndColor(0.375f, "_RGB_150_0_0");
        royBigBL.addScalarAndColor(0.250f, "_RGB_100_0_0");
        royBigBL.addScalarAndColor(0.125f, "_RGB_60_0_0");
        royBigBL.addScalarAndColor(0.000f, "_black");
        royBigBL.addScalarAndColor(-0.125f, "_RGB_0_0_80");
        royBigBL.addScalarAndColor(-0.250f, "_RGB_0_0_170");
        royBigBL.addScalarAndColor(-0.375f, "_RGB_75_0_125");
        royBigBL.addScalarAndColor(-0.500f, "_RGB_125_0_160");
        royBigBL.addScalarAndColor(-0.625f, "_RGB_75_125_0");
        royBigBL.addScalarAndColor(-0.750f, "_RGB_0_200_0");
        royBigBL.addScalarAndColor(-0.875f, "_RGB_0_255_0");
        royBigBL.addScalarAndColor(-0.990f, "_RGB_0_255_255");
        royBigBL.addScalarAndColor(-1.00f, "_RGB_0_255_255");

        addPalette(royBigBL);
    }

    //----------------------------------------------------------------------
    // Orange-Yellow palette
    //
    if (this->getPaletteByName("Orange-Yellow") == NULL) {
        this->addColor("_oy1",  0, 0, 0 );
        this->addColor("_oy2",  130, 2, 0 );
        this->addColor("_oy3",  254, 130, 2 );
        this->addColor("_oy4",  254, 254, 126 );
        this->addColor("_oy5",  254, 254, 254 );

        Palette orangeYellow;
        orangeYellow.setName("Orange-Yellow");
        orangeYellow.addScalarAndColor( 1.0f, "_oy5");
        orangeYellow.addScalarAndColor( 0.5f, "_oy4");
        orangeYellow.addScalarAndColor( 0.0f, "_oy3");
        orangeYellow.addScalarAndColor(-0.5f, "_oy2");
        orangeYellow.addScalarAndColor(-1.0f, "_oy1");
        addPalette(orangeYellow);
    }

    //
    // Create a palette with just white and black designed to be used
    // with the interpolate option
    //
    if (this->getPaletteByName(Palette::GRAY_INTERP_PALETTE_NAME) == NULL) {
        this->addColor("_white_gray_interp",  255, 255, 255 );
        this->addColor("_black_gray_interp",  0, 0, 0 );


        Palette palGrayPositiveInterp;
        palGrayPositiveInterp.setName(Palette::GRAY_INTERP_POSITIVE_PALETTE_NAME);
        palGrayPositiveInterp.addScalarAndColor( 1.0f, "_white_gray_interp");
        palGrayPositiveInterp.addScalarAndColor(0.0f, "_black_gray_interp");
        addPalette(palGrayPositiveInterp);

        Palette palGrayInterp;
        palGrayInterp.setName(Palette::GRAY_INTERP_PALETTE_NAME);
        palGrayInterp.addScalarAndColor( 1.0f, "_white_gray_interp");
        palGrayInterp.addScalarAndColor(-1.0f, "_black_gray_interp");
        addPalette(palGrayInterp);
    }

    //------------------------------------------------------------------------
    //
    // Palette by David Van Essen
    //
    int oran_yell[3] = { 0xff, 0x99, 0x00 };
    this->addColor("_oran-yell", oran_yell);
    int red[3] = { 0xff, 0x00, 0x00 };
    this->addColor("_red", red);
    int cyan[3] = { 0x00, 0xff, 0xff };
    this->addColor("_cyan", cyan);
    int green[3] = { 0x00, 0xff, 0x00 };
    this->addColor("_green", green);
    int limegreen[3] = { 0x10, 0xb0, 0x10 };
    this->addColor("_limegreen", limegreen);
    int violet[3] = { 0xe2, 0x51, 0xe2 };
    this->addColor("_violet", violet);
    int hotpink[3] = { 0xff, 0x38, 0x8d };
    this->addColor("_hotpink", hotpink);
    int white[3] = { 0xff, 0xff, 0xff };
    this->addColor("_white", white);
    int gry_dd[3] = { 0xdd, 0xdd, 0xdd };
    this->addColor("_gry-dd", gry_dd );
    int gry_bb[3] = { 0xbb, 0xbb, 0xbb };
    this->addColor("_gry-bb", gry_bb);
    int purple2[3] = { 0x66, 0x00, 0x33 };
    this->addColor("_purple2", purple2);
    int blue_videen11[3] = { 0x33, 0x33, 0x4c };
    this->addColor("_blue_videen11", blue_videen11);
    int blue_videen9[3] = { 0x4c, 0x4c, 0x7f };
    this->addColor("_blue_videen9", blue_videen9);
    int blue_videen7[3] = { 0x7f, 0x7f, 0xcc };
    this->addColor("_blue_videen7", blue_videen7);

    if (this->getPaletteByName("clear_brain") == NULL) {
        Palette clearBrain;
        clearBrain.setName("clear_brain");
        clearBrain.addScalarAndColor(1.0f , "_red");
        clearBrain.addScalarAndColor(0.9f , "_orange");
        clearBrain.addScalarAndColor(0.8f , "_oran-yell");
        clearBrain.addScalarAndColor(0.7f , "_yellow");
        clearBrain.addScalarAndColor(0.6f , "_limegreen");
        clearBrain.addScalarAndColor(0.5f , "_green");
        clearBrain.addScalarAndColor(0.4f , "_blue_videen7");
        clearBrain.addScalarAndColor(0.3f , "_blue_videen9");
        clearBrain.addScalarAndColor(0.2f , "_blue_videen11");
        clearBrain.addScalarAndColor(0.1f , "_purple2");
        clearBrain.addScalarAndColor(0.0f , "none");
        clearBrain.addScalarAndColor(-0.1f , "_cyan");
        clearBrain.addScalarAndColor(-0.2f , "_green");
        clearBrain.addScalarAndColor(-0.3f , "_limegreen");
        clearBrain.addScalarAndColor(-0.4f , "_violet");
        clearBrain.addScalarAndColor(-0.5f , "_hotpink");
        clearBrain.addScalarAndColor(-0.6f , "_white");
        clearBrain.addScalarAndColor(-0.7f , "_gry-dd");
        clearBrain.addScalarAndColor(-0.8f , "_gry-bb");
        clearBrain.addScalarAndColor(-0.9f , "_black");
        addPalette(clearBrain);
    }
    if (this->getPaletteByName("videen_style") == NULL) {
        Palette videenStyle;
        videenStyle.setName("videen_style");
        videenStyle.addScalarAndColor(1.0f, "_red");
        videenStyle.addScalarAndColor(0.9f, "_orange");
        videenStyle.addScalarAndColor(0.8f, "_oran-yell");
        videenStyle.addScalarAndColor(0.7f, "_yellow");
        videenStyle.addScalarAndColor(0.6f, "_limegreen");
        videenStyle.addScalarAndColor(0.5f, "_green");
        videenStyle.addScalarAndColor(0.4f, "_blue_videen7");
        videenStyle.addScalarAndColor(0.3f, "_blue_videen9");
        videenStyle.addScalarAndColor(0.2f, "_blue_videen11");
        videenStyle.addScalarAndColor(0.1f, "_purple2");
        videenStyle.addScalarAndColor(0.0f, "_black");
        videenStyle.addScalarAndColor(-0.1f, "_cyan");
        videenStyle.addScalarAndColor(-0.2f, "_green");
        videenStyle.addScalarAndColor(-0.3f, "_limegreen");
        videenStyle.addScalarAndColor(-0.4f, "_violet");
        videenStyle.addScalarAndColor(-0.5f, "_hotpink");
        videenStyle.addScalarAndColor(-0.6f, "_white");
        videenStyle.addScalarAndColor(-0.7f, "_gry-dd");
        videenStyle.addScalarAndColor(-0.8f, "_gry-bb");
        videenStyle.addScalarAndColor(-0.9f, "_black");
        addPalette(videenStyle);
    }

    if (this->getPaletteByName("fidl") == NULL) {
        int Bright_Yellow[3] = { 0xee, 0xee, 0x55 };
        this->addColor("_Bright_Yellow", Bright_Yellow);
        int Mustard[3] = { 0xdd, 0xdd, 0x66 };
        this->addColor("_Mustard", Mustard);
        int Brown_Mustard[3] = { 0xdd, 0x99, 0x00 };
        this->addColor("_Brown_Mustard", Brown_Mustard);
        int Bright_Red[3] = { 0xff, 0x00, 0x00 };
        this->addColor("_Bright_Red", Bright_Red);
        int Fire_Engine_Red[3] = { 0xdd, 0x00, 0x00 };
        this->addColor("_Fire_Engine_Red", Fire_Engine_Red);
        int Brick[3] = { 0xbb, 0x00, 0x00 };
        this->addColor("_Brick", Brick);
        int Beet[3] = { 0x99, 0x00, 0x00 };
        this->addColor("_Beet", Beet);
        int Beaujolais[3] = { 0x77, 0x00, 0x00 };
        this->addColor("_Beaujolais", Beaujolais);
        int Burgundy[3] = { 0x55, 0x00, 0x00 };
        this->addColor("_Burgundy", Burgundy);
        int Thrombin[3] = { 0x11, 0x00, 0x00 };
        this->addColor("_Thrombin", Thrombin);
        int Deep_Green[3] = { 0x00, 0x11, 0x00 };
        this->addColor("_Deep_Green", Deep_Green);
        int British_Racing_Green[3] = { 0x00, 0x55, 0x00 };
        this->addColor("_British_Racing_Green", British_Racing_Green);
        int Kelp[3] = { 0x00, 0x77, 0x00 };
        this->addColor("_Kelp", Kelp);
        int Lime[3] = { 0x00, 0x99, 0x00 };
        this->addColor("_Lime", Lime);
        int Mint[3] = { 0x00, 0xbb, 0x00 };
        this->addColor("_Mint", Mint);
        int Brussell_Sprout[3] = { 0x00, 0xdd, 0x00 };
        this->addColor("_Brussell_Sprout", Brussell_Sprout);
        int Bright_Green[3] = { 0x00, 0xff, 0x00 };
        this->addColor("_Bright_Green", Bright_Green);
        int Periwinkle[3] = { 0x66, 0x66, 0xbb };
        this->addColor("_Periwinkle", Periwinkle);
        int Azure[3] = { 0x88, 0x88, 0xee };
        this->addColor("_Azure", Azure);
        int Turquoise[3] = { 0x00, 0xcc, 0xcc };
        this->addColor("_Turquoise", Turquoise);

        Palette fidl;
        fidl.setName("fidl");
        fidl.addScalarAndColor(1.0f, "_Bright_Yellow");
        fidl.addScalarAndColor(0.9f, "_Mustard");
        fidl.addScalarAndColor(0.8f, "_Brown_Mustard");
        fidl.addScalarAndColor(0.7f, "_Bright_Red");
        fidl.addScalarAndColor(0.6f, "_Fire_Engine_Red");
        fidl.addScalarAndColor(0.5f, "_Brick");
        fidl.addScalarAndColor(0.4f, "_Beet");
        fidl.addScalarAndColor(0.3f, "_Beaujolais");
        fidl.addScalarAndColor(0.2f, "_Burgundy");
        fidl.addScalarAndColor(0.1f, "_Thrombin");
        fidl.addScalarAndColor(0.0f, "none");
        fidl.addScalarAndColor(-0.1f, "_Deep_Green");
        fidl.addScalarAndColor(-0.2f, "_British_Racing_Green");
        fidl.addScalarAndColor(-0.3f, "_Kelp");
        fidl.addScalarAndColor(-0.4f, "_Lime");
        fidl.addScalarAndColor(-0.5f, "_Mint");
        fidl.addScalarAndColor(-0.6f, "_Brussell_Sprout");
        fidl.addScalarAndColor(-0.7f, "_Bright_Green");
        fidl.addScalarAndColor(-0.8f, "_Periwinkle");
        fidl.addScalarAndColor(-0.9f, "_Azure");
        fidl.addScalarAndColor(-1.0f, "_Turquoise");
        addPalette(fidl);
    }

    //------------------------------------------------------------------------
    //
    // Colors by Russ H.
    //
    int _rbgyr20_10[3] = { 0x00, 0xff, 0x00 };
    this->addColor("_rbgyr20_10", _rbgyr20_10);
    int _rbgyr20_15[3] = { 0xff, 0xff, 0x00 };
    this->addColor("_rbgyr20_15", _rbgyr20_15);
    int _rbgyr20_20[3] = { 0xff, 0x00, 0x00 };
    this->addColor("_rbgyr20_20", _rbgyr20_20);

    int _rbgyr20_21[3] = { 0x9d, 0x22, 0xc1 };
    this->addColor("_rbgyr20_21", _rbgyr20_21);
    int _rbgyr20_22[3] = { 0x81, 0x06, 0xa5 };
    this->addColor("_rbgyr20_22", _rbgyr20_22);
    int _rbgyr20_23[3] = { 0xff, 0xec, 0x00 };
    this->addColor("_rbgyr20_23", _rbgyr20_23);
    int _rbgyr20_24[3] = { 0xff, 0xd6, 0x00 };
    this->addColor("_rbgyr20_24", _rbgyr20_24);
    int _rbgyr20_25[3] = { 0xff, 0xbc, 0x00 };
    this->addColor("_rbgyr20_25", _rbgyr20_25);
    int _rbgyr20_26[3] = { 0xff, 0x9c, 0x00 };
    this->addColor("_rbgyr20_26", _rbgyr20_26);
    int _rbgyr20_27[3] = { 0xff, 0x7c, 0x00 };
    this->addColor("_rbgyr20_27", _rbgyr20_27);
    int _rbgyr20_28[3] = { 0xff, 0x5c, 0x00 };
    this->addColor("_rbgyr20_28", _rbgyr20_28);
    int _rbgyr20_29[3] = { 0xff, 0x3d, 0x00 };
    this->addColor("_rbgyr20_29", _rbgyr20_29);
    int _rbgyr20_30[3] = { 0xff, 0x23, 0x00 };
    this->addColor("_rbgyr20_30", _rbgyr20_30);
    int _rbgyr20_31[3] = { 0x00, 0xed, 0x12 };
    this->addColor("_rbgyr20_31", _rbgyr20_31);
    int _rbgyr20_32[3] = { 0x00, 0xd5, 0x2a };
    this->addColor("_rbgyr20_32", _rbgyr20_32);
    int _rbgyr20_33[3] = { 0x00, 0xb9, 0x46 };
    this->addColor("_rbgyr20_33", _rbgyr20_33);
    int _rbgyr20_34[3] = { 0x00, 0x9b, 0x64 };
    this->addColor("_rbgyr20_34", _rbgyr20_34);
    int _rbgyr20_35[3] = { 0x00, 0x7b, 0x84 };
    this->addColor("_rbgyr20_35", _rbgyr20_35);
    int _rbgyr20_36[3] = { 0x00, 0x5b, 0xa4 };
    this->addColor("_rbgyr20_36", _rbgyr20_36);
    int _rbgyr20_37[3] = { 0x00, 0x44, 0xbb };
    this->addColor("_rbgyr20_37", _rbgyr20_37);
    int _rbgyr20_38[3] = { 0x00, 0x24, 0xdb };
    this->addColor("_rbgyr20_38", _rbgyr20_38);
    int _rbgyr20_39[3] = { 0x00, 0x00, 0xff };
    this->addColor("_rbgyr20_39", _rbgyr20_39);

    int _rbgyr20_40[3] = { 0xff, 0xf1, 0x00 };
    this->addColor("_rbgyr20_40", _rbgyr20_40);
    int _rbgyr20_41[3] = { 0xff, 0xdc, 0x00 };
    this->addColor("_rbgyr20_41", _rbgyr20_41);
    int _rbgyr20_42[3] = { 0xff, 0xcb, 0x00 };
    this->addColor("_rbgyr20_42", _rbgyr20_42);
    int _rbgyr20_43[3] = { 0xff, 0xc2, 0x00 };
    this->addColor("_rbgyr20_43", _rbgyr20_43);
    int _rbgyr20_44[3] = { 0xff, 0xae, 0x00 };
    this->addColor("_rbgyr20_44", _rbgyr20_44);
    int _rbgyr20_45[3] = { 0xff, 0x9f, 0x00 };
    this->addColor("_rbgyr20_45", _rbgyr20_45);
    int _rbgyr20_46[3] = { 0xff, 0x86, 0x00 };
    this->addColor("_rbgyr20_46", _rbgyr20_46);
    int _rbgyr20_47[3] = { 0xff, 0x59, 0x00 };
    this->addColor("_rbgyr20_47", _rbgyr20_47);
    int _rbgyr20_48[3] = { 0x00, 0xff, 0x2d };
    this->addColor("_rbgyr20_48", _rbgyr20_48);
    int _rbgyr20_49[3] = { 0x00, 0xff, 0x65 };
    this->addColor("_rbgyr20_49", _rbgyr20_49);
    int _rbgyr20_50[3] = { 0x00, 0xff, 0xa5 };
    this->addColor("_rbgyr20_50", _rbgyr20_50);
    int _rbgyr20_51[3] = { 0x00, 0xff, 0xdd };
    this->addColor("_rbgyr20_51", _rbgyr20_51);
    int _rbgyr20_52[3] = { 0x00, 0xff, 0xff };
    this->addColor("_rbgyr20_52", _rbgyr20_52);
    int _rbgyr20_53[3] = { 0x00, 0xe9, 0xff };
    this->addColor("_rbgyr20_53", _rbgyr20_53);
    int _rbgyr20_54[3] = { 0x00, 0xad, 0xff };
    this->addColor("_rbgyr20_54", _rbgyr20_54);
    int _rbgyr20_55[3] = { 0x00, 0x69, 0xff };
    this->addColor("_rbgyr20_55", _rbgyr20_55);
    int _rbgyr20_56[3] = { 0xff, 0x00, 0xb9 };
    this->addColor("_rbgyr20_56", _rbgyr20_56);
    int _rbgyr20_57[3] = { 0xff, 0x00, 0x63 };
    this->addColor("_rbgyr20_57", _rbgyr20_57);
    int _rbgyr20_58[3] = { 0xff, 0x05, 0x00 };
    this->addColor("_rbgyr20_58", _rbgyr20_58);
    int _rbgyr20_59[3] = { 0xff, 0x32, 0x00 };
    this->addColor("_rbgyr20_59", _rbgyr20_59);
    int _rbgyr20_60[3] =  { 0xff, 0x70, 0x00 };
    this->addColor("_rbgyr20_60", _rbgyr20_60);
    int _rbgyr20_61[3] = { 0xff, 0xa4, 0x00 };
    this->addColor("_rbgyr20_61", _rbgyr20_61);
    int _rbgyr20_62[3] = { 0xff, 0xba, 0x00 };
    this->addColor("_rbgyr20_62", _rbgyr20_62);
    int _rbgyr20_63[3] = { 0xff, 0xd3, 0x00 };
    this->addColor("_rbgyr20_63", _rbgyr20_63);
    int _rbgyr20_64[3] = { 0x42, 0x21, 0xdb };
    this->addColor("_rbgyr20_64", _rbgyr20_64);
    int _rbgyr20_65[3] = { 0x10, 0x08, 0xf6 };
    this->addColor("_rbgyr20_65", _rbgyr20_65);
    int _rbgyr20_66[3] = { 0x00, 0x13, 0xff };
    this->addColor("_rbgyr20_66", _rbgyr20_66);
    int _rbgyr20_67[3] = { 0x00, 0x5b, 0xff };
    this->addColor("_rbgyr20_67", _rbgyr20_67);
    int _rbgyr20_68[3] = { 0x00, 0xb3, 0xff };
    this->addColor("_rbgyr20_68", _rbgyr20_68);
    int _rbgyr20_69[3] = { 0x00, 0xfc, 0xff };
    this->addColor("_rbgyr20_69", _rbgyr20_69);
    int _rbgyr20_70[3] = { 0x00, 0xff, 0xcd };
    this->addColor("_rbgyr20_70", _rbgyr20_70);
    int _rbgyr20_71[3] = { 0x00, 0xff, 0x74 };
    this->addColor("_rbgyr20_71", _rbgyr20_71);
    int _rbgyr20_72[3] = { 0xff, 0x00, 0xf9 };
    this->addColor("_rbgyr20_72", _rbgyr20_72);
    int _rbgyr20_73[3] = { 0x62, 0x31, 0xc9 };
    this->addColor("_rbgyr20_73", _rbgyr20_73);

    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("raich4_clrmid") == NULL) {
        Palette r4;
        r4.setName("raich4_clrmid");
        r4.addScalarAndColor(1.000000f, "_rbgyr20_20");
        r4.addScalarAndColor(0.900000f, "_rbgyr20_30");
        r4.addScalarAndColor(0.800000f, "_rbgyr20_29");
        r4.addScalarAndColor(0.700000f, "_rbgyr20_28");
        r4.addScalarAndColor(0.600000f, "_rbgyr20_27");
        r4.addScalarAndColor(0.500000f, "_rbgyr20_26");
        r4.addScalarAndColor(0.400000f, "_rbgyr20_25");
        r4.addScalarAndColor(0.300000f, "_rbgyr20_24");
        r4.addScalarAndColor(0.200000f, "_rbgyr20_23");
        r4.addScalarAndColor(0.100000f, "_rbgyr20_15");
        r4.addScalarAndColor(0.000000f, "none");
        r4.addScalarAndColor(-0.100000f, "_rbgyr20_10");
        r4.addScalarAndColor(-0.200000f, "_rbgyr20_31");
        r4.addScalarAndColor(-0.300000f, "_rbgyr20_32");
        r4.addScalarAndColor(-0.400000f, "_rbgyr20_33");
        r4.addScalarAndColor(-0.500000f, "_rbgyr20_34");
        r4.addScalarAndColor(-0.600000f, "_rbgyr20_35");
        r4.addScalarAndColor(-0.700000f, "_rbgyr20_36");
        r4.addScalarAndColor(-0.800000f, "_rbgyr20_37");
        r4.addScalarAndColor(-0.900000f, "_rbgyr20_38");
        r4.addScalarAndColor(-1.000000f, "_rbgyr20_39");
        addPalette(r4);
    }

    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("raich6_clrmid") == NULL) {
        Palette r6;
        r6.setName("raich6_clrmid");
        r6.addScalarAndColor(1.000000f, "_rbgyr20_20");
        r6.addScalarAndColor(0.900000f, "_rbgyr20_47");
        r6.addScalarAndColor(0.800000f, "_rbgyr20_46");
        r6.addScalarAndColor(0.700000f, "_rbgyr20_45");
        r6.addScalarAndColor(0.600000f, "_rbgyr20_44");
        r6.addScalarAndColor(0.500000f, "_rbgyr20_43");
        r6.addScalarAndColor(0.400000f, "_rbgyr20_42");
        r6.addScalarAndColor(0.300000f, "_rbgyr20_41");
        r6.addScalarAndColor(0.200000f, "_rbgyr20_40");
        r6.addScalarAndColor(0.100000f, "_rbgyr20_15");
        r6.addScalarAndColor(0.000000f, "none");
        r6.addScalarAndColor(-0.100000f, "_rbgyr20_10");
        r6.addScalarAndColor(-0.200000f, "_rbgyr20_48");
        r6.addScalarAndColor(-0.300000f, "_rbgyr20_49");
        r6.addScalarAndColor(-0.400000f, "_rbgyr20_50");
        r6.addScalarAndColor(-0.500000f, "_rbgyr20_51");
        r6.addScalarAndColor(-0.600000f, "_rbgyr20_52");
        r6.addScalarAndColor(-0.700000f, "_rbgyr20_53");
        r6.addScalarAndColor(-0.800000f, "_rbgyr20_54");
        r6.addScalarAndColor(-0.900000f, "_rbgyr20_55");
        r6.addScalarAndColor(-1.000000f, "_rbgyr20_39");
        addPalette(r6);
    }

    //------------------------------------------------------------------------
    //
    // Palette by Russ H.
    //
    if (this->getPaletteByName("HSB8_clrmid") == NULL) {
        Palette hsb8;
        hsb8.setName("HSB8_clrmid");
        hsb8.addScalarAndColor(1.000000f, "_rbgyr20_15");
        hsb8.addScalarAndColor(0.900000f, "_rbgyr20_63");
        hsb8.addScalarAndColor(0.800000f, "_rbgyr20_62");
        hsb8.addScalarAndColor(0.700000f, "_rbgyr20_61");
        hsb8.addScalarAndColor(0.600000f, "_rbgyr20_60");
        hsb8.addScalarAndColor(0.500000f, "_rbgyr20_59");
        hsb8.addScalarAndColor(0.400000f, "_rbgyr20_58");
        hsb8.addScalarAndColor(0.300000f, "_rbgyr20_57");
        hsb8.addScalarAndColor(0.200000f, "_rbgyr20_56");
        hsb8.addScalarAndColor(0.100000f, "_rbgyr20_72");
        hsb8.addScalarAndColor(0.000000f, "none");
        hsb8.addScalarAndColor(-0.100000f, "_rbgyr20_73");
        hsb8.addScalarAndColor(-0.200000f, "_rbgyr20_64");
        hsb8.addScalarAndColor(-0.300000f, "_rbgyr20_65");
        hsb8.addScalarAndColor(-0.400000f, "_rbgyr20_66");
        hsb8.addScalarAndColor(-0.500000f, "_rbgyr20_67");
        hsb8.addScalarAndColor(-0.600000f, "_rbgyr20_68");
        hsb8.addScalarAndColor(-0.700000f, "_rbgyr20_69");
        hsb8.addScalarAndColor(-0.800000f, "_rbgyr20_70");
        hsb8.addScalarAndColor(-0.900000f, "_rbgyr20_71");
        hsb8.addScalarAndColor(-1.000000f, "_rbgyr20_10");
        addPalette(hsb8);
    }

    //------------------------------------------------------------------------
    //
    // Palette by Daniel MArgulies
    //
    int mymap0[3] = { 0x48, 0x23, 0x74 };
    this->addColor("_mymap0", mymap0);
    int mymap1[3] = { 0x47, 0x25, 0x75 };
    this->addColor("_mymap1", mymap1);
    int mymap2[3] = { 0x47, 0x26, 0x76 };
    this->addColor("_mymap2", mymap2);
    int mymap3[3] = { 0x47, 0x27, 0x77 };
    this->addColor("_mymap3", mymap3);
    int mymap4[3] = { 0x47, 0x28, 0x78 };
    this->addColor("_mymap4", mymap4);
    int mymap5[3] = { 0x47, 0x2a, 0x79 };
    this->addColor("_mymap5", mymap5);
    int mymap6[3] = { 0x47, 0x2b, 0x7a };
    this->addColor("_mymap6", mymap6);
    int mymap7[3] = { 0x47, 0x2c, 0x7b };
    this->addColor("_mymap7", mymap7);
    int mymap8[3] = { 0x46, 0x2d, 0x7c };
    this->addColor("_mymap8", mymap8);
    int mymap9[3] = { 0x46, 0x2f, 0x7c };
    this->addColor("_mymap9", mymap9);
    int mymap10[3] = { 0x46, 0x30, 0x7d };
    this->addColor("_mymap10", mymap10);
    int mymap11[3] = { 0x46, 0x31, 0x7e };
    this->addColor("_mymap11", mymap11);
    int mymap12[3] = { 0x45, 0x32, 0x7f };
    this->addColor("_mymap12", mymap12);
    int mymap13[3] = { 0x45, 0x34, 0x7f };
    this->addColor("_mymap13", mymap13);
    int mymap14[3] = { 0x45, 0x35, 0x80 };
    this->addColor("_mymap14", mymap14);
    int mymap15[3] = { 0x45, 0x36, 0x81 };
    this->addColor("_mymap15", mymap15);
    int mymap16[3] = { 0x44, 0x37, 0x81 };
    this->addColor("_mymap16", mymap16);
    int mymap17[3] = { 0x44, 0x39, 0x82 };
    this->addColor("_mymap17", mymap17);
    int mymap18[3] = { 0x43, 0x3a, 0x83 };
    this->addColor("_mymap18", mymap18);
    int mymap19[3] = { 0x43, 0x3b, 0x83 };
    this->addColor("_mymap19", mymap19);
    int mymap20[3] = { 0x43, 0x3c, 0x84 };
    this->addColor("_mymap20", mymap20);
    int mymap21[3] = { 0x42, 0x3d, 0x84 };
    this->addColor("_mymap21", mymap21);
    int mymap22[3] = { 0x42, 0x3e, 0x85 };
    this->addColor("_mymap22", mymap22);
    int mymap23[3] = { 0x42, 0x40, 0x85 };
    this->addColor("_mymap23", mymap23);
    int mymap24[3] = { 0x41, 0x41, 0x86 };
    this->addColor("_mymap24", mymap24);
    int mymap25[3] = { 0x41, 0x42, 0x86 };
    this->addColor("_mymap25", mymap25);
    int mymap26[3] = { 0x40, 0x43, 0x87 };
    this->addColor("_mymap26", mymap26);
    int mymap27[3] = { 0x40, 0x44, 0x87 };
    this->addColor("_mymap27", mymap27);
    int mymap28[3] = { 0x3f, 0x45, 0x87 };
    this->addColor("_mymap28", mymap28);
    int mymap29[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymap29", mymap29);
    int mymap30[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymap30", mymap30);
    int mymap31[3] = { 0x3e, 0x48, 0x88 };
    this->addColor("_mymap31", mymap31);
    int mymap32[3] = { 0x3e, 0x49, 0x89 };
    this->addColor("_mymap32", mymap32);
    int mymap33[3] = { 0x3d, 0x4a, 0x89 };
    this->addColor("_mymap33", mymap33);
    int mymap34[3] = { 0x3d, 0x4b, 0x89 };
    this->addColor("_mymap34", mymap34);
    int mymap35[3] = { 0x3d, 0x4c, 0x89 };
    this->addColor("_mymap35", mymap35);
    int mymap36[3] = { 0x3c, 0x4d, 0x8a };
    this->addColor("_mymap36", mymap36);
    int mymap37[3] = { 0x3c, 0x4e, 0x8a };
    this->addColor("_mymap37", mymap37);
    int mymap38[3] = { 0x3b, 0x50, 0x8a };
    this->addColor("_mymap38", mymap38);
    int mymap39[3] = { 0x3b, 0x51, 0x8a };
    this->addColor("_mymap39", mymap39);
    int mymap40[3] = { 0x3a, 0x52, 0x8b };
    this->addColor("_mymap40", mymap40);
    int mymap41[3] = { 0x3a, 0x53, 0x8b };
    this->addColor("_mymap41", mymap41);
    int mymap42[3] = { 0x39, 0x54, 0x8b };
    this->addColor("_mymap42", mymap42);
    int mymap43[3] = { 0x39, 0x55, 0x8b };
    this->addColor("_mymap43", mymap43);
    int mymap44[3] = { 0x38, 0x56, 0x8b };
    this->addColor("_mymap44", mymap44);
    int mymap45[3] = { 0x38, 0x57, 0x8c };
    this->addColor("_mymap45", mymap45);
    int mymap46[3] = { 0x37, 0x58, 0x8c };
    this->addColor("_mymap46", mymap46);
    int mymap47[3] = { 0x37, 0x59, 0x8c };
    this->addColor("_mymap47", mymap47);
    int mymap48[3] = { 0x36, 0x5a, 0x8c };
    this->addColor("_mymap48", mymap48);
    int mymap49[3] = { 0x36, 0x5b, 0x8c };
    this->addColor("_mymap49", mymap49);
    int mymap50[3] = { 0x35, 0x5c, 0x8c };
    this->addColor("_mymap50", mymap50);
    int mymap51[3] = { 0x35, 0x5d, 0x8c };
    this->addColor("_mymap51", mymap51);
    int mymap52[3] = { 0x34, 0x5e, 0x8d };
    this->addColor("_mymap52", mymap52);
    int mymap53[3] = { 0x34, 0x5f, 0x8d };
    this->addColor("_mymap53", mymap53);
    int mymap54[3] = { 0x33, 0x60, 0x8d };
    this->addColor("_mymap54", mymap54);
    int mymap55[3] = { 0x33, 0x61, 0x8d };
    this->addColor("_mymap55", mymap55);
    int mymap56[3] = { 0x32, 0x62, 0x8d };
    this->addColor("_mymap56", mymap56);
    int mymap57[3] = { 0x32, 0x63, 0x8d };
    this->addColor("_mymap57", mymap57);
    int mymap58[3] = { 0x31, 0x64, 0x8d };
    this->addColor("_mymap58", mymap58);
    int mymap59[3] = { 0x31, 0x65, 0x8d };
    this->addColor("_mymap59", mymap59);
    int mymap60[3] = { 0x31, 0x66, 0x8d };
    this->addColor("_mymap60", mymap60);
    int mymap61[3] = { 0x30, 0x67, 0x8d };
    this->addColor("_mymap61", mymap61);
    int mymap62[3] = { 0x30, 0x68, 0x8d };
    this->addColor("_mymap62", mymap62);
    int mymap63[3] = { 0x2f, 0x69, 0x8d };
    this->addColor("_mymap63", mymap63);
    int mymap64[3] = { 0x2f, 0x6a, 0x8d };
    this->addColor("_mymap64", mymap64);
    int mymap65[3] = { 0x2e, 0x6b, 0x8e };
    this->addColor("_mymap65", mymap65);
    int mymap66[3] = { 0x2e, 0x6c, 0x8e };
    this->addColor("_mymap66", mymap66);
    int mymap67[3] = { 0x2e, 0x6d, 0x8e };
    this->addColor("_mymap67", mymap67);
    int mymap68[3] = { 0x2d, 0x6e, 0x8e };
    this->addColor("_mymap68", mymap68);
    int mymap69[3] = { 0x2d, 0x6f, 0x8e };
    this->addColor("_mymap69", mymap69);
    int mymap70[3] = { 0x2c, 0x70, 0x8e };
    this->addColor("_mymap70", mymap70);
    int mymap71[3] = { 0x2c, 0x71, 0x8e };
    this->addColor("_mymap71", mymap71);
    int mymap72[3] = { 0x2c, 0x72, 0x8e };
    this->addColor("_mymap72", mymap72);
    int mymap73[3] = { 0x2b, 0x73, 0x8e };
    this->addColor("_mymap73", mymap73);
    int mymap74[3] = { 0x2b, 0x74, 0x8e };
    this->addColor("_mymap74", mymap74);
    int mymap75[3] = { 0x2a, 0x75, 0x8e };
    this->addColor("_mymap75", mymap75);
    int mymap76[3] = { 0x2a, 0x76, 0x8e };
    this->addColor("_mymap76", mymap76);
    int mymap77[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymap77", mymap77);
    int mymap78[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymap78", mymap78);
    int mymap79[3] = { 0x29, 0x78, 0x8e };
    this->addColor("_mymap79", mymap79);
    int mymap80[3] = { 0x29, 0x79, 0x8e };
    this->addColor("_mymap80", mymap80);
    int mymap81[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymap81", mymap81);
    int mymap82[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymap82", mymap82);
    int mymap83[3] = { 0x28, 0x7b, 0x8e };
    this->addColor("_mymap83", mymap83);
    int mymap84[3] = { 0x27, 0x7c, 0x8e };
    this->addColor("_mymap84", mymap84);
    int mymap85[3] = { 0x27, 0x7d, 0x8e };
    this->addColor("_mymap85", mymap85);
    int mymap86[3] = { 0x27, 0x7e, 0x8e };
    this->addColor("_mymap86", mymap86);
    int mymap87[3] = { 0x26, 0x7f, 0x8e };
    this->addColor("_mymap87", mymap87);
    int mymap88[3] = { 0x26, 0x80, 0x8e };
    this->addColor("_mymap88", mymap88);
    int mymap89[3] = { 0x26, 0x81, 0x8e };
    this->addColor("_mymap89", mymap89);
    int mymap90[3] = { 0x25, 0x82, 0x8e };
    this->addColor("_mymap90", mymap90);
    int mymap91[3] = { 0x25, 0x83, 0x8d };
    this->addColor("_mymap91", mymap91);
    int mymap92[3] = { 0x24, 0x84, 0x8d };
    this->addColor("_mymap92", mymap92);
    int mymap93[3] = { 0x24, 0x85, 0x8d };
    this->addColor("_mymap93", mymap93);
    int mymap94[3] = { 0x24, 0x86, 0x8d };
    this->addColor("_mymap94", mymap94);
    int mymap95[3] = { 0x23, 0x87, 0x8d };
    this->addColor("_mymap95", mymap95);
    int mymap96[3] = { 0x23, 0x88, 0x8d };
    this->addColor("_mymap96", mymap96);
    int mymap97[3] = { 0x23, 0x89, 0x8d };
    this->addColor("_mymap97", mymap97);
    int mymap98[3] = { 0x22, 0x89, 0x8d };
    this->addColor("_mymap98", mymap98);
    int mymap99[3] = { 0x22, 0x8a, 0x8d };
    this->addColor("_mymap99", mymap99);
    int mymap100[3] = { 0x22, 0x8b, 0x8d };
    this->addColor("_mymap100", mymap100);
    int mymap101[3] = { 0x21, 0x8c, 0x8d };
    this->addColor("_mymap101", mymap101);
    int mymap102[3] = { 0x21, 0x8d, 0x8c };
    this->addColor("_mymap102", mymap102);
    int mymap103[3] = { 0x21, 0x8e, 0x8c };
    this->addColor("_mymap103", mymap103);
    int mymap104[3] = { 0x20, 0x8f, 0x8c };
    this->addColor("_mymap104", mymap104);
    int mymap105[3] = { 0x20, 0x90, 0x8c };
    this->addColor("_mymap105", mymap105);
    int mymap106[3] = { 0x20, 0x91, 0x8c };
    this->addColor("_mymap106", mymap106);
    int mymap107[3] = { 0x1f, 0x92, 0x8c };
    this->addColor("_mymap107", mymap107);
    int mymap108[3] = { 0x1f, 0x93, 0x8b };
    this->addColor("_mymap108", mymap108);
    int mymap109[3] = { 0x1f, 0x94, 0x8b };
    this->addColor("_mymap109", mymap109);
    int mymap110[3] = { 0x1f, 0x95, 0x8b };
    this->addColor("_mymap110", mymap110);
    int mymap111[3] = { 0x1f, 0x96, 0x8b };
    this->addColor("_mymap111", mymap111);
    int mymap112[3] = { 0x1e, 0x97, 0x8a };
    this->addColor("_mymap112", mymap112);
    int mymap113[3] = { 0x1e, 0x98, 0x8a };
    this->addColor("_mymap113", mymap113);
    int mymap114[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymap114", mymap114);
    int mymap115[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymap115", mymap115);
    int mymap116[3] = { 0x1e, 0x9a, 0x89 };
    this->addColor("_mymap116", mymap116);
    int mymap117[3] = { 0x1e, 0x9b, 0x89 };
    this->addColor("_mymap117", mymap117);
    int mymap118[3] = { 0x1e, 0x9c, 0x89 };
    this->addColor("_mymap118", mymap118);
    int mymap119[3] = { 0x1e, 0x9d, 0x88 };
    this->addColor("_mymap119", mymap119);
    int mymap120[3] = { 0x1e, 0x9e, 0x88 };
    this->addColor("_mymap120", mymap120);
    int mymap121[3] = { 0x1e, 0x9f, 0x88 };
    this->addColor("_mymap121", mymap121);
    int mymap122[3] = { 0x1e, 0xa0, 0x87 };
    this->addColor("_mymap122", mymap122);
    int mymap123[3] = { 0x1f, 0xa1, 0x87 };
    this->addColor("_mymap123", mymap123);
    int mymap124[3] = { 0x1f, 0xa2, 0x86 };
    this->addColor("_mymap124", mymap124);
    int mymap125[3] = { 0x1f, 0xa3, 0x86 };
    this->addColor("_mymap125", mymap125);
    int mymap126[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymap126", mymap126);
    int mymap127[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymap127", mymap127);
    int mymap128[3] = { 0x20, 0xa5, 0x85 };
    this->addColor("_mymap128", mymap128);
    int mymap129[3] = { 0x21, 0xa6, 0x85 };
    this->addColor("_mymap129", mymap129);
    int mymap130[3] = { 0x21, 0xa7, 0x84 };
    this->addColor("_mymap130", mymap130);
    int mymap131[3] = { 0x22, 0xa7, 0x84 };
    this->addColor("_mymap131", mymap131);
    int mymap132[3] = { 0x23, 0xa8, 0x83 };
    this->addColor("_mymap132", mymap132);
    int mymap133[3] = { 0x23, 0xa9, 0x82 };
    this->addColor("_mymap133", mymap133);
    int mymap134[3] = { 0x24, 0xaa, 0x82 };
    this->addColor("_mymap134", mymap134);
    int mymap135[3] = { 0x25, 0xab, 0x81 };
    this->addColor("_mymap135", mymap135);
    int mymap136[3] = { 0x26, 0xac, 0x81 };
    this->addColor("_mymap136", mymap136);
    int mymap137[3] = { 0x27, 0xad, 0x80 };
    this->addColor("_mymap137", mymap137);
    int mymap138[3] = { 0x28, 0xae, 0x7f };
    this->addColor("_mymap138", mymap138);
    int mymap139[3] = { 0x29, 0xaf, 0x7f };
    this->addColor("_mymap139", mymap139);
    int mymap140[3] = { 0x2a, 0xb0, 0x7e };
    this->addColor("_mymap140", mymap140);
    int mymap141[3] = { 0x2b, 0xb1, 0x7d };
    this->addColor("_mymap141", mymap141);
    int mymap142[3] = { 0x2c, 0xb1, 0x7d };
    this->addColor("_mymap142", mymap142);
    int mymap143[3] = { 0x2e, 0xb2, 0x7c };
    this->addColor("_mymap143", mymap143);
    int mymap144[3] = { 0x2f, 0xb3, 0x7b };
    this->addColor("_mymap144", mymap144);
    int mymap145[3] = { 0x30, 0xb4, 0x7a };
    this->addColor("_mymap145", mymap145);
    int mymap146[3] = { 0x32, 0xb5, 0x7a };
    this->addColor("_mymap146", mymap146);
    int mymap147[3] = { 0x33, 0xb6, 0x79 };
    this->addColor("_mymap147", mymap147);
    int mymap148[3] = { 0x35, 0xb7, 0x78 };
    this->addColor("_mymap148", mymap148);
    int mymap149[3] = { 0x36, 0xb8, 0x77 };
    this->addColor("_mymap149", mymap149);
    int mymap150[3] = { 0x38, 0xb9, 0x76 };
    this->addColor("_mymap150", mymap150);
    int mymap151[3] = { 0x39, 0xb9, 0x76 };
    this->addColor("_mymap151", mymap151);
    int mymap152[3] = { 0x3b, 0xba, 0x75 };
    this->addColor("_mymap152", mymap152);
    int mymap153[3] = { 0x3d, 0xbb, 0x74 };
    this->addColor("_mymap153", mymap153);
    int mymap154[3] = { 0x3e, 0xbc, 0x73 };
    this->addColor("_mymap154", mymap154);
    int mymap155[3] = { 0x40, 0xbd, 0x72 };
    this->addColor("_mymap155", mymap155);
    int mymap156[3] = { 0x42, 0xbe, 0x71 };
    this->addColor("_mymap156", mymap156);
    int mymap157[3] = { 0x44, 0xbe, 0x70 };
    this->addColor("_mymap157", mymap157);
    int mymap158[3] = { 0x45, 0xbf, 0x6f };
    this->addColor("_mymap158", mymap158);
    int mymap159[3] = { 0x47, 0xc0, 0x6e };
    this->addColor("_mymap159", mymap159);
    int mymap160[3] = { 0x49, 0xc1, 0x6d };
    this->addColor("_mymap160", mymap160);
    int mymap161[3] = { 0x4b, 0xc2, 0x6c };
    this->addColor("_mymap161", mymap161);
    int mymap162[3] = { 0x4d, 0xc2, 0x6b };
    this->addColor("_mymap162", mymap162);
    int mymap163[3] = { 0x4f, 0xc3, 0x69 };
    this->addColor("_mymap163", mymap163);
    int mymap164[3] = { 0x51, 0xc4, 0x68 };
    this->addColor("_mymap164", mymap164);
    int mymap165[3] = { 0x53, 0xc5, 0x67 };
    this->addColor("_mymap165", mymap165);
    int mymap166[3] = { 0x55, 0xc6, 0x66 };
    this->addColor("_mymap166", mymap166);
    int mymap167[3] = { 0x57, 0xc6, 0x65 };
    this->addColor("_mymap167", mymap167);
    int mymap168[3] = { 0x59, 0xc7, 0x64 };
    this->addColor("_mymap168", mymap168);
    int mymap169[3] = { 0x5b, 0xc8, 0x62 };
    this->addColor("_mymap169", mymap169);
    int mymap170[3] = { 0x5e, 0xc9, 0x61 };
    this->addColor("_mymap170", mymap170);
    int mymap171[3] = { 0x60, 0xc9, 0x60 };
    this->addColor("_mymap171", mymap171);
    int mymap172[3] = { 0x62, 0xca, 0x5f };
    this->addColor("_mymap172", mymap172);
    int mymap173[3] = { 0x64, 0xcb, 0x5d };
    this->addColor("_mymap173", mymap173);
    int mymap174[3] = { 0x67, 0xcc, 0x5c };
    this->addColor("_mymap174", mymap174);
    int mymap175[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymap175", mymap175);
    int mymap176[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymap176", mymap176);
    int mymap177[3] = { 0x6b, 0xcd, 0x59 };
    this->addColor("_mymap177", mymap177);
    int mymap178[3] = { 0x6d, 0xce, 0x58 };
    this->addColor("_mymap178", mymap178);
    int mymap179[3] = { 0x70, 0xce, 0x56 };
    this->addColor("_mymap179", mymap179);
    int mymap180[3] = { 0x72, 0xcf, 0x55 };
    this->addColor("_mymap180", mymap180);
    int mymap181[3] = { 0x74, 0xd0, 0x54 };
    this->addColor("_mymap181", mymap181);
    int mymap182[3] = { 0x77, 0xd0, 0x52 };
    this->addColor("_mymap182", mymap182);
    int mymap183[3] = { 0x79, 0xd1, 0x51 };
    this->addColor("_mymap183", mymap183);
    int mymap184[3] = { 0x7c, 0xd2, 0x4f };
    this->addColor("_mymap184", mymap184);
    int mymap185[3] = { 0x7e, 0xd2, 0x4e };
    this->addColor("_mymap185", mymap185);
    int mymap186[3] = { 0x81, 0xd3, 0x4c };
    this->addColor("_mymap186", mymap186);
    int mymap187[3] = { 0x83, 0xd3, 0x4b };
    this->addColor("_mymap187", mymap187);
    int mymap188[3] = { 0x86, 0xd4, 0x49 };
    this->addColor("_mymap188", mymap188);
    int mymap189[3] = { 0x88, 0xd5, 0x47 };
    this->addColor("_mymap189", mymap189);
    int mymap190[3] = { 0x8b, 0xd5, 0x46 };
    this->addColor("_mymap190", mymap190);
    int mymap191[3] = { 0x8d, 0xd6, 0x44 };
    this->addColor("_mymap191", mymap191);
    int mymap192[3] = { 0x90, 0xd6, 0x43 };
    this->addColor("_mymap192", mymap192);
    int mymap193[3] = { 0x92, 0xd7, 0x41 };
    this->addColor("_mymap193", mymap193);
    int mymap194[3] = { 0x95, 0xd7, 0x3f };
    this->addColor("_mymap194", mymap194);
    int mymap195[3] = { 0x97, 0xd8, 0x3e };
    this->addColor("_mymap195", mymap195);
    int mymap196[3] = { 0x9a, 0xd8, 0x3c };
    this->addColor("_mymap196", mymap196);
    int mymap197[3] = { 0x9d, 0xd9, 0x3a };
    this->addColor("_mymap197", mymap197);
    int mymap198[3] = { 0x9f, 0xd9, 0x38 };
    this->addColor("_mymap198", mymap198);
    int mymap199[3] = { 0xa2, 0xda, 0x37 };
    this->addColor("_mymap199", mymap199);
    int mymap200[3] = { 0xa5, 0xda, 0x35 };
    this->addColor("_mymap200", mymap200);
    int mymap201[3] = { 0xa7, 0xdb, 0x33 };
    this->addColor("_mymap201", mymap201);
    int mymap202[3] = { 0xaa, 0xdb, 0x32 };
    this->addColor("_mymap202", mymap202);
    int mymap203[3] = { 0xad, 0xdc, 0x30 };
    this->addColor("_mymap203", mymap203);
    int mymap204[3] = { 0xaf, 0xdc, 0x2e };
    this->addColor("_mymap204", mymap204);
    int mymap205[3] = { 0xb2, 0xdd, 0x2c };
    this->addColor("_mymap205", mymap205);
    int mymap206[3] = { 0xb5, 0xdd, 0x2b };
    this->addColor("_mymap206", mymap206);
    int mymap207[3] = { 0xb7, 0xdd, 0x29 };
    this->addColor("_mymap207", mymap207);
    int mymap208[3] = { 0xba, 0xde, 0x27 };
    this->addColor("_mymap208", mymap208);
    int mymap209[3] = { 0xbd, 0xde, 0x26 };
    this->addColor("_mymap209", mymap209);
    int mymap210[3] = { 0xbf, 0xdf, 0x24 };
    this->addColor("_mymap210", mymap210);
    int mymap211[3] = { 0xc2, 0xdf, 0x22 };
    this->addColor("_mymap211", mymap211);
    int mymap212[3] = { 0xc5, 0xdf, 0x21 };
    this->addColor("_mymap212", mymap212);
    int mymap213[3] = { 0xc7, 0xe0, 0x1f };
    this->addColor("_mymap213", mymap213);
    int mymap214[3] = { 0xca, 0xe0, 0x1e };
    this->addColor("_mymap214", mymap214);
    int mymap215[3] = { 0xcd, 0xe0, 0x1d };
    this->addColor("_mymap215", mymap215);
    int mymap216[3] = { 0xcf, 0xe1, 0x1c };
    this->addColor("_mymap216", mymap216);
    int mymap217[3] = { 0xd2, 0xe1, 0x1b };
    this->addColor("_mymap217", mymap217);
    int mymap218[3] = { 0xd4, 0xe1, 0x1a };
    this->addColor("_mymap218", mymap218);
    int mymap219[3] = { 0xd7, 0xe2, 0x19 };
    this->addColor("_mymap219", mymap219);
    int mymap220[3] = { 0xda, 0xe2, 0x18 };
    this->addColor("_mymap220", mymap220);
    int mymap221[3] = { 0xdc, 0xe2, 0x18 };
    this->addColor("_mymap221", mymap221);
    int mymap222[3] = { 0xdf, 0xe3, 0x18 };
    this->addColor("_mymap222", mymap222);
    int mymap223[3] = { 0xe1, 0xe3, 0x18 };
    this->addColor("_mymap223", mymap223);
    int mymap224[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymap224", mymap224);
    int mymap225[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymap225", mymap225);
    int mymap226[3] = { 0xe7, 0xe4, 0x19 };
    this->addColor("_mymap226", mymap226);
    int mymap227[3] = { 0xe9, 0xe4, 0x19 };
    this->addColor("_mymap227", mymap227);
    int mymap228[3] = { 0xec, 0xe4, 0x1a };
    this->addColor("_mymap228", mymap228);
    int mymap229[3] = { 0xee, 0xe5, 0x1b };
    this->addColor("_mymap229", mymap229);
    int mymap230[3] = { 0xf1, 0xe5, 0x1c };
    this->addColor("_mymap230", mymap230);
    int mymap231[3] = { 0xfe, 0xe2, 0x90 };
    this->addColor("_mymap231", mymap231);
    int mymap232[3] = { 0xfe, 0xda, 0x7f };
    this->addColor("_mymap232", mymap232);
    int mymap233[3] = { 0xfe, 0xd3, 0x6f };
    this->addColor("_mymap233", mymap233);
    int mymap234[3] = { 0xfe, 0xcb, 0x5e };
    this->addColor("_mymap234", mymap234);
    int mymap235[3] = { 0xfe, 0xc3, 0x4e };
    this->addColor("_mymap235", mymap235);
    int mymap236[3] = { 0xfe, 0xb8, 0x45 };
    this->addColor("_mymap236", mymap236);
    int mymap237[3] = { 0xfe, 0xad, 0x3b };
    this->addColor("_mymap237", mymap237);
    int mymap238[3] = { 0xfe, 0xa3, 0x31 };
    this->addColor("_mymap238", mymap238);
    int mymap239[3] = { 0xfd, 0x98, 0x28 };
    this->addColor("_mymap239", mymap239);
    int mymap240[3] = { 0xf9, 0x8e, 0x23 };
    this->addColor("_mymap240", mymap240);
    int mymap241[3] = { 0xf4, 0x83, 0x1e };
    this->addColor("_mymap241", mymap241);
    int mymap242[3] = { 0xf0, 0x79, 0x18 };
    this->addColor("_mymap242", mymap242);
    int mymap243[3] = { 0xeb, 0x6f, 0x13 };
    this->addColor("_mymap243", mymap243);
    int mymap244[3] = { 0xe3, 0x66, 0xf };
    this->addColor("_mymap244", mymap244);
    int mymap245[3] = { 0xdb, 0x5d, 0xa };
    this->addColor("_mymap245", mymap245);
    int mymap246[3] = { 0xd3, 0x54, 0x6 };
    this->addColor("_mymap246", mymap246);
    int mymap247[3] = { 0xca, 0x4b, 0x2 };
    this->addColor("_mymap247", mymap247);
    int mymap248[3] = { 0xbe, 0x45, 0x2 };
    this->addColor("_mymap248", mymap248);
    int mymap249[3] = { 0xb1, 0x3f, 0x3 };
    this->addColor("_mymap249", mymap249);
    int mymap250[3] = { 0xa4, 0x39, 0x3 };
    this->addColor("_mymap250", mymap250);
    int mymap251[3] = { 0x97, 0x33, 0x4 };
    this->addColor("_mymap251", mymap251);
    int mymap252[3] = { 0x8a, 0x2f, 0x4 };
    this->addColor("_mymap252", mymap252);
    int mymap253[3] = { 0x7e, 0x2c, 0x5 };
    this->addColor("_mymap253", mymap253);
    int mymap254[3] = { 0x71, 0x28, 0x5 };
    this->addColor("_mymap254", mymap254);
    int mymap255[3] = { 0x66, 0x25, 0x6 };
    this->addColor("_mymap255", mymap255);
    if (this->getPaletteByName("margulies") == NULL) {
        Palette mymap;
        mymap.setName("margulies");
        mymap.addScalarAndColor( 1.000000f, "_mymap0");
        mymap.addScalarAndColor( 0.992157f, "_mymap1");
        mymap.addScalarAndColor( 0.984314f, "_mymap2");
        mymap.addScalarAndColor( 0.976471f, "_mymap3");
        mymap.addScalarAndColor( 0.968627f, "_mymap4");
        mymap.addScalarAndColor( 0.960784f, "_mymap5");
        mymap.addScalarAndColor( 0.952941f, "_mymap6");
        mymap.addScalarAndColor( 0.945098f, "_mymap7");
        mymap.addScalarAndColor( 0.937255f, "_mymap8");
        mymap.addScalarAndColor( 0.929412f, "_mymap9");
        mymap.addScalarAndColor( 0.921569f, "_mymap10");
        mymap.addScalarAndColor( 0.913725f, "_mymap11");
        mymap.addScalarAndColor( 0.905882f, "_mymap12");
        mymap.addScalarAndColor( 0.898039f, "_mymap13");
        mymap.addScalarAndColor( 0.890196f, "_mymap14");
        mymap.addScalarAndColor( 0.882353f, "_mymap15");
        mymap.addScalarAndColor( 0.874510f, "_mymap16");
        mymap.addScalarAndColor( 0.866667f, "_mymap17");
        mymap.addScalarAndColor( 0.858824f, "_mymap18");
        mymap.addScalarAndColor( 0.850980f, "_mymap19");
        mymap.addScalarAndColor( 0.843137f, "_mymap20");
        mymap.addScalarAndColor( 0.835294f, "_mymap21");
        mymap.addScalarAndColor( 0.827451f, "_mymap22");
        mymap.addScalarAndColor( 0.819608f, "_mymap23");
        mymap.addScalarAndColor( 0.811765f, "_mymap24");
        mymap.addScalarAndColor( 0.803922f, "_mymap25");
        mymap.addScalarAndColor( 0.796078f, "_mymap26");
        mymap.addScalarAndColor( 0.788235f, "_mymap27");
        mymap.addScalarAndColor( 0.780392f, "_mymap28");
        mymap.addScalarAndColor( 0.772549f, "_mymap29");
        mymap.addScalarAndColor( 0.764706f, "_mymap30");
        mymap.addScalarAndColor( 0.756863f, "_mymap31");
        mymap.addScalarAndColor( 0.749020f, "_mymap32");
        mymap.addScalarAndColor( 0.741176f, "_mymap33");
        mymap.addScalarAndColor( 0.733333f, "_mymap34");
        mymap.addScalarAndColor( 0.725490f, "_mymap35");
        mymap.addScalarAndColor( 0.717647f, "_mymap36");
        mymap.addScalarAndColor( 0.709804f, "_mymap37");
        mymap.addScalarAndColor( 0.701961f, "_mymap38");
        mymap.addScalarAndColor( 0.694118f, "_mymap39");
        mymap.addScalarAndColor( 0.686275f, "_mymap40");
        mymap.addScalarAndColor( 0.678431f, "_mymap41");
        mymap.addScalarAndColor( 0.670588f, "_mymap42");
        mymap.addScalarAndColor( 0.662745f, "_mymap43");
        mymap.addScalarAndColor( 0.654902f, "_mymap44");
        mymap.addScalarAndColor( 0.647059f, "_mymap45");
        mymap.addScalarAndColor( 0.639216f, "_mymap46");
        mymap.addScalarAndColor( 0.631373f, "_mymap47");
        mymap.addScalarAndColor( 0.623529f, "_mymap48");
        mymap.addScalarAndColor( 0.615686f, "_mymap49");
        mymap.addScalarAndColor( 0.607843f, "_mymap50");
        mymap.addScalarAndColor( 0.600000f, "_mymap51");
        mymap.addScalarAndColor( 0.592157f, "_mymap52");
        mymap.addScalarAndColor( 0.584314f, "_mymap53");
        mymap.addScalarAndColor( 0.576471f, "_mymap54");
        mymap.addScalarAndColor( 0.568627f, "_mymap55");
        mymap.addScalarAndColor( 0.560784f, "_mymap56");
        mymap.addScalarAndColor( 0.552941f, "_mymap57");
        mymap.addScalarAndColor( 0.545098f, "_mymap58");
        mymap.addScalarAndColor( 0.537255f, "_mymap59");
        mymap.addScalarAndColor( 0.529412f, "_mymap60");
        mymap.addScalarAndColor( 0.521569f, "_mymap61");
        mymap.addScalarAndColor( 0.513725f, "_mymap62");
        mymap.addScalarAndColor( 0.505882f, "_mymap63");
        mymap.addScalarAndColor( 0.498039f, "_mymap64");
        mymap.addScalarAndColor( 0.490196f, "_mymap65");
        mymap.addScalarAndColor( 0.482353f, "_mymap66");
        mymap.addScalarAndColor( 0.474510f, "_mymap67");
        mymap.addScalarAndColor( 0.466667f, "_mymap68");
        mymap.addScalarAndColor( 0.458824f, "_mymap69");
        mymap.addScalarAndColor( 0.450980f, "_mymap70");
        mymap.addScalarAndColor( 0.443137f, "_mymap71");
        mymap.addScalarAndColor( 0.435294f, "_mymap72");
        mymap.addScalarAndColor( 0.427451f, "_mymap73");
        mymap.addScalarAndColor( 0.419608f, "_mymap74");
        mymap.addScalarAndColor( 0.411765f, "_mymap75");
        mymap.addScalarAndColor( 0.403922f, "_mymap76");
        mymap.addScalarAndColor( 0.396078f, "_mymap77");
        mymap.addScalarAndColor( 0.388235f, "_mymap78");
        mymap.addScalarAndColor( 0.380392f, "_mymap79");
        mymap.addScalarAndColor( 0.372549f, "_mymap80");
        mymap.addScalarAndColor( 0.364706f, "_mymap81");
        mymap.addScalarAndColor( 0.356863f, "_mymap82");
        mymap.addScalarAndColor( 0.349020f, "_mymap83");
        mymap.addScalarAndColor( 0.341176f, "_mymap84");
        mymap.addScalarAndColor( 0.333333f, "_mymap85");
        mymap.addScalarAndColor( 0.325490f, "_mymap86");
        mymap.addScalarAndColor( 0.317647f, "_mymap87");
        mymap.addScalarAndColor( 0.309804f, "_mymap88");
        mymap.addScalarAndColor( 0.301961f, "_mymap89");
        mymap.addScalarAndColor( 0.294118f, "_mymap90");
        mymap.addScalarAndColor( 0.286275f, "_mymap91");
        mymap.addScalarAndColor( 0.278431f, "_mymap92");
        mymap.addScalarAndColor( 0.270588f, "_mymap93");
        mymap.addScalarAndColor( 0.262745f, "_mymap94");
        mymap.addScalarAndColor( 0.254902f, "_mymap95");
        mymap.addScalarAndColor( 0.247059f, "_mymap96");
        mymap.addScalarAndColor( 0.239216f, "_mymap97");
        mymap.addScalarAndColor( 0.231373f, "_mymap98");
        mymap.addScalarAndColor( 0.223529f, "_mymap99");
        mymap.addScalarAndColor( 0.215686f, "_mymap100");
        mymap.addScalarAndColor( 0.207843f, "_mymap101");
        mymap.addScalarAndColor( 0.200000f, "_mymap102");
        mymap.addScalarAndColor( 0.192157f, "_mymap103");
        mymap.addScalarAndColor( 0.184314f, "_mymap104");
        mymap.addScalarAndColor( 0.176471f, "_mymap105");
        mymap.addScalarAndColor( 0.168627f, "_mymap106");
        mymap.addScalarAndColor( 0.160784f, "_mymap107");
        mymap.addScalarAndColor( 0.152941f, "_mymap108");
        mymap.addScalarAndColor( 0.145098f, "_mymap109");
        mymap.addScalarAndColor( 0.137255f, "_mymap110");
        mymap.addScalarAndColor( 0.129412f, "_mymap111");
        mymap.addScalarAndColor( 0.121569f, "_mymap112");
        mymap.addScalarAndColor( 0.113725f, "_mymap113");
        mymap.addScalarAndColor( 0.105882f, "_mymap114");
        mymap.addScalarAndColor( 0.098039f, "_mymap115");
        mymap.addScalarAndColor( 0.090196f, "_mymap116");
        mymap.addScalarAndColor( 0.082353f, "_mymap117");
        mymap.addScalarAndColor( 0.074510f, "_mymap118");
        mymap.addScalarAndColor( 0.066667f, "_mymap119");
        mymap.addScalarAndColor( 0.058824f, "_mymap120");
        mymap.addScalarAndColor( 0.050980f, "_mymap121");
        mymap.addScalarAndColor( 0.043137f, "_mymap122");
        mymap.addScalarAndColor( 0.035294f, "_mymap123");
        mymap.addScalarAndColor( 0.027451f, "_mymap124");
        mymap.addScalarAndColor( 0.019608f, "_mymap125");
        mymap.addScalarAndColor( 0.011765f, "_mymap126");
        mymap.addScalarAndColor( 0.003922f, "_mymap127");
        mymap.addScalarAndColor( -0.003922f, "_mymap128");
        mymap.addScalarAndColor( -0.011765f, "_mymap129");
        mymap.addScalarAndColor( -0.019608f, "_mymap130");
        mymap.addScalarAndColor( -0.027451f, "_mymap131");
        mymap.addScalarAndColor( -0.035294f, "_mymap132");
        mymap.addScalarAndColor( -0.043137f, "_mymap133");
        mymap.addScalarAndColor( -0.050980f, "_mymap134");
        mymap.addScalarAndColor( -0.058824f, "_mymap135");
        mymap.addScalarAndColor( -0.066667f, "_mymap136");
        mymap.addScalarAndColor( -0.074510f, "_mymap137");
        mymap.addScalarAndColor( -0.082353f, "_mymap138");
        mymap.addScalarAndColor( -0.090196f, "_mymap139");
        mymap.addScalarAndColor( -0.098039f, "_mymap140");
        mymap.addScalarAndColor( -0.105882f, "_mymap141");
        mymap.addScalarAndColor( -0.113725f, "_mymap142");
        mymap.addScalarAndColor( -0.121569f, "_mymap143");
        mymap.addScalarAndColor( -0.129412f, "_mymap144");
        mymap.addScalarAndColor( -0.137255f, "_mymap145");
        mymap.addScalarAndColor( -0.145098f, "_mymap146");
        mymap.addScalarAndColor( -0.152941f, "_mymap147");
        mymap.addScalarAndColor( -0.160784f, "_mymap148");
        mymap.addScalarAndColor( -0.168627f, "_mymap149");
        mymap.addScalarAndColor( -0.176471f, "_mymap150");
        mymap.addScalarAndColor( -0.184314f, "_mymap151");
        mymap.addScalarAndColor( -0.192157f, "_mymap152");
        mymap.addScalarAndColor( -0.200000f, "_mymap153");
        mymap.addScalarAndColor( -0.207843f, "_mymap154");
        mymap.addScalarAndColor( -0.215686f, "_mymap155");
        mymap.addScalarAndColor( -0.223529f, "_mymap156");
        mymap.addScalarAndColor( -0.231373f, "_mymap157");
        mymap.addScalarAndColor( -0.239216f, "_mymap158");
        mymap.addScalarAndColor( -0.247059f, "_mymap159");
        mymap.addScalarAndColor( -0.254902f, "_mymap160");
        mymap.addScalarAndColor( -0.262745f, "_mymap161");
        mymap.addScalarAndColor( -0.270588f, "_mymap162");
        mymap.addScalarAndColor( -0.278431f, "_mymap163");
        mymap.addScalarAndColor( -0.286275f, "_mymap164");
        mymap.addScalarAndColor( -0.294118f, "_mymap165");
        mymap.addScalarAndColor( -0.301961f, "_mymap166");
        mymap.addScalarAndColor( -0.309804f, "_mymap167");
        mymap.addScalarAndColor( -0.317647f, "_mymap168");
        mymap.addScalarAndColor( -0.325490f, "_mymap169");
        mymap.addScalarAndColor( -0.333333f, "_mymap170");
        mymap.addScalarAndColor( -0.341176f, "_mymap171");
        mymap.addScalarAndColor( -0.349020f, "_mymap172");
        mymap.addScalarAndColor( -0.356863f, "_mymap173");
        mymap.addScalarAndColor( -0.364706f, "_mymap174");
        mymap.addScalarAndColor( -0.372549f, "_mymap175");
        mymap.addScalarAndColor( -0.380392f, "_mymap176");
        mymap.addScalarAndColor( -0.388235f, "_mymap177");
        mymap.addScalarAndColor( -0.396078f, "_mymap178");
        mymap.addScalarAndColor( -0.403922f, "_mymap179");
        mymap.addScalarAndColor( -0.411765f, "_mymap180");
        mymap.addScalarAndColor( -0.419608f, "_mymap181");
        mymap.addScalarAndColor( -0.427451f, "_mymap182");
        mymap.addScalarAndColor( -0.435294f, "_mymap183");
        mymap.addScalarAndColor( -0.443137f, "_mymap184");
        mymap.addScalarAndColor( -0.450980f, "_mymap185");
        mymap.addScalarAndColor( -0.458824f, "_mymap186");
        mymap.addScalarAndColor( -0.466667f, "_mymap187");
        mymap.addScalarAndColor( -0.474510f, "_mymap188");
        mymap.addScalarAndColor( -0.482353f, "_mymap189");
        mymap.addScalarAndColor( -0.490196f, "_mymap190");
        mymap.addScalarAndColor( -0.498039f, "_mymap191");
        mymap.addScalarAndColor( -0.505882f, "_mymap192");
        mymap.addScalarAndColor( -0.513725f, "_mymap193");
        mymap.addScalarAndColor( -0.521569f, "_mymap194");
        mymap.addScalarAndColor( -0.529412f, "_mymap195");
        mymap.addScalarAndColor( -0.537255f, "_mymap196");
        mymap.addScalarAndColor( -0.545098f, "_mymap197");
        mymap.addScalarAndColor( -0.552941f, "_mymap198");
        mymap.addScalarAndColor( -0.560784f, "_mymap199");
        mymap.addScalarAndColor( -0.568627f, "_mymap200");
        mymap.addScalarAndColor( -0.576471f, "_mymap201");
        mymap.addScalarAndColor( -0.584314f, "_mymap202");
        mymap.addScalarAndColor( -0.592157f, "_mymap203");
        mymap.addScalarAndColor( -0.600000f, "_mymap204");
        mymap.addScalarAndColor( -0.607843f, "_mymap205");
        mymap.addScalarAndColor( -0.615686f, "_mymap206");
        mymap.addScalarAndColor( -0.623529f, "_mymap207");
        mymap.addScalarAndColor( -0.631373f, "_mymap208");
        mymap.addScalarAndColor( -0.639216f, "_mymap209");
        mymap.addScalarAndColor( -0.647059f, "_mymap210");
        mymap.addScalarAndColor( -0.654902f, "_mymap211");
        mymap.addScalarAndColor( -0.662745f, "_mymap212");
        mymap.addScalarAndColor( -0.670588f, "_mymap213");
        mymap.addScalarAndColor( -0.678431f, "_mymap214");
        mymap.addScalarAndColor( -0.686275f, "_mymap215");
        mymap.addScalarAndColor( -0.694118f, "_mymap216");
        mymap.addScalarAndColor( -0.701961f, "_mymap217");
        mymap.addScalarAndColor( -0.709804f, "_mymap218");
        mymap.addScalarAndColor( -0.717647f, "_mymap219");
        mymap.addScalarAndColor( -0.725490f, "_mymap220");
        mymap.addScalarAndColor( -0.733333f, "_mymap221");
        mymap.addScalarAndColor( -0.741176f, "_mymap222");
        mymap.addScalarAndColor( -0.749020f, "_mymap223");
        mymap.addScalarAndColor( -0.756863f, "_mymap224");
        mymap.addScalarAndColor( -0.764706f, "_mymap225");
        mymap.addScalarAndColor( -0.772549f, "_mymap226");
        mymap.addScalarAndColor( -0.780392f, "_mymap227");
        mymap.addScalarAndColor( -0.788235f, "_mymap228");
        mymap.addScalarAndColor( -0.796078f, "_mymap229");
        mymap.addScalarAndColor( -0.803922f, "_mymap230");
        mymap.addScalarAndColor( -0.811765f, "_mymap231");
        mymap.addScalarAndColor( -0.819608f, "_mymap232");
        mymap.addScalarAndColor( -0.827451f, "_mymap233");
        mymap.addScalarAndColor( -0.835294f, "_mymap234");
        mymap.addScalarAndColor( -0.843137f, "_mymap235");
        mymap.addScalarAndColor( -0.850980f, "_mymap236");
        mymap.addScalarAndColor( -0.858824f, "_mymap237");
        mymap.addScalarAndColor( -0.866667f, "_mymap238");
        mymap.addScalarAndColor( -0.874510f, "_mymap239");
        mymap.addScalarAndColor( -0.882353f, "_mymap240");
        mymap.addScalarAndColor( -0.890196f, "_mymap241");
        mymap.addScalarAndColor( -0.898039f, "_mymap242");
        mymap.addScalarAndColor( -0.905882f, "_mymap243");
        mymap.addScalarAndColor( -0.913725f, "_mymap244");
        mymap.addScalarAndColor( -0.921569f, "_mymap245");
        mymap.addScalarAndColor( -0.929412f, "_mymap246");
        mymap.addScalarAndColor( -0.937255f, "_mymap247");
        mymap.addScalarAndColor( -0.945098f, "_mymap248");
        mymap.addScalarAndColor( -0.952941f, "_mymap249");
        mymap.addScalarAndColor( -0.960784f, "_mymap250");
        mymap.addScalarAndColor( -0.968627f, "_mymap251");
        mymap.addScalarAndColor( -0.976471f, "_mymap252");
        mymap.addScalarAndColor( -0.984314f, "_mymap253");
        mymap.addScalarAndColor( -0.992157f, "_mymap254");
        mymap.addScalarAndColor( -1.000000f, "_mymap255");
        addPalette(mymap);
    }
    int mymapInv0[3] = { 0x66, 0x25, 0x6 };
    this->addColor("_mymapInv0", mymapInv0);
    int mymapInv1[3] = { 0x71, 0x28, 0x5 };
    this->addColor("_mymapInv1", mymapInv1);
    int mymapInv2[3] = { 0x7e, 0x2c, 0x5 };
    this->addColor("_mymapInv2", mymapInv2);
    int mymapInv3[3] = { 0x8a, 0x2f, 0x4 };
    this->addColor("_mymapInv3", mymapInv3);
    int mymapInv4[3] = { 0x97, 0x33, 0x4 };
    this->addColor("_mymapInv4", mymapInv4);
    int mymapInv5[3] = { 0xa4, 0x39, 0x3 };
    this->addColor("_mymapInv5", mymapInv5);
    int mymapInv6[3] = { 0xb1, 0x3f, 0x3 };
    this->addColor("_mymapInv6", mymapInv6);
    int mymapInv7[3] = { 0xbe, 0x45, 0x2 };
    this->addColor("_mymapInv7", mymapInv7);
    int mymapInv8[3] = { 0xca, 0x4b, 0x2 };
    this->addColor("_mymapInv8", mymapInv8);
    int mymapInv9[3] = { 0xd3, 0x54, 0x6 };
    this->addColor("_mymapInv9", mymapInv9);
    int mymapInv10[3] = { 0xdb, 0x5d, 0xa };
    this->addColor("_mymapInv10", mymapInv10);
    int mymapInv11[3] = { 0xe3, 0x66, 0xf };
    this->addColor("_mymapInv11", mymapInv11);
    int mymapInv12[3] = { 0xeb, 0x6f, 0x13 };
    this->addColor("_mymapInv12", mymapInv12);
    int mymapInv13[3] = { 0xf0, 0x79, 0x18 };
    this->addColor("_mymapInv13", mymapInv13);
    int mymapInv14[3] = { 0xf4, 0x83, 0x1e };
    this->addColor("_mymapInv14", mymapInv14);
    int mymapInv15[3] = { 0xf9, 0x8e, 0x23 };
    this->addColor("_mymapInv15", mymapInv15);
    int mymapInv16[3] = { 0xfd, 0x98, 0x28 };
    this->addColor("_mymapInv16", mymapInv16);
    int mymapInv17[3] = { 0xfe, 0xa3, 0x31 };
    this->addColor("_mymapInv17", mymapInv17);
    int mymapInv18[3] = { 0xfe, 0xad, 0x3b };
    this->addColor("_mymapInv18", mymapInv18);
    int mymapInv19[3] = { 0xfe, 0xb8, 0x45 };
    this->addColor("_mymapInv19", mymapInv19);
    int mymapInv20[3] = { 0xfe, 0xc3, 0x4e };
    this->addColor("_mymapInv20", mymapInv20);
    int mymapInv21[3] = { 0xfe, 0xcb, 0x5e };
    this->addColor("_mymapInv21", mymapInv21);
    int mymapInv22[3] = { 0xfe, 0xd3, 0x6f };
    this->addColor("_mymapInv22", mymapInv22);
    int mymapInv23[3] = { 0xfe, 0xda, 0x7f };
    this->addColor("_mymapInv23", mymapInv23);
    int mymapInv24[3] = { 0xfe, 0xe2, 0x90 };
    this->addColor("_mymapInv24", mymapInv24);
    int mymapInv25[3] = { 0xf1, 0xe5, 0x1c };
    this->addColor("_mymapInv25", mymapInv25);
    int mymapInv26[3] = { 0xee, 0xe5, 0x1b };
    this->addColor("_mymapInv26", mymapInv26);
    int mymapInv27[3] = { 0xec, 0xe4, 0x1a };
    this->addColor("_mymapInv27", mymapInv27);
    int mymapInv28[3] = { 0xe9, 0xe4, 0x19 };
    this->addColor("_mymapInv28", mymapInv28);
    int mymapInv29[3] = { 0xe7, 0xe4, 0x19 };
    this->addColor("_mymapInv29", mymapInv29);
    int mymapInv30[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapInv30", mymapInv30);
    int mymapInv31[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapInv31", mymapInv31);
    int mymapInv32[3] = { 0xe1, 0xe3, 0x18 };
    this->addColor("_mymapInv32", mymapInv32);
    int mymapInv33[3] = { 0xdf, 0xe3, 0x18 };
    this->addColor("_mymapInv33", mymapInv33);
    int mymapInv34[3] = { 0xdc, 0xe2, 0x18 };
    this->addColor("_mymapInv34", mymapInv34);
    int mymapInv35[3] = { 0xda, 0xe2, 0x18 };
    this->addColor("_mymapInv35", mymapInv35);
    int mymapInv36[3] = { 0xd7, 0xe2, 0x19 };
    this->addColor("_mymapInv36", mymapInv36);
    int mymapInv37[3] = { 0xd4, 0xe1, 0x1a };
    this->addColor("_mymapInv37", mymapInv37);
    int mymapInv38[3] = { 0xd2, 0xe1, 0x1b };
    this->addColor("_mymapInv38", mymapInv38);
    int mymapInv39[3] = { 0xcf, 0xe1, 0x1c };
    this->addColor("_mymapInv39", mymapInv39);
    int mymapInv40[3] = { 0xcd, 0xe0, 0x1d };
    this->addColor("_mymapInv40", mymapInv40);
    int mymapInv41[3] = { 0xca, 0xe0, 0x1e };
    this->addColor("_mymapInv41", mymapInv41);
    int mymapInv42[3] = { 0xc7, 0xe0, 0x1f };
    this->addColor("_mymapInv42", mymapInv42);
    int mymapInv43[3] = { 0xc5, 0xdf, 0x21 };
    this->addColor("_mymapInv43", mymapInv43);
    int mymapInv44[3] = { 0xc2, 0xdf, 0x22 };
    this->addColor("_mymapInv44", mymapInv44);
    int mymapInv45[3] = { 0xbf, 0xdf, 0x24 };
    this->addColor("_mymapInv45", mymapInv45);
    int mymapInv46[3] = { 0xbd, 0xde, 0x26 };
    this->addColor("_mymapInv46", mymapInv46);
    int mymapInv47[3] = { 0xba, 0xde, 0x27 };
    this->addColor("_mymapInv47", mymapInv47);
    int mymapInv48[3] = { 0xb7, 0xdd, 0x29 };
    this->addColor("_mymapInv48", mymapInv48);
    int mymapInv49[3] = { 0xb5, 0xdd, 0x2b };
    this->addColor("_mymapInv49", mymapInv49);
    int mymapInv50[3] = { 0xb2, 0xdd, 0x2c };
    this->addColor("_mymapInv50", mymapInv50);
    int mymapInv51[3] = { 0xaf, 0xdc, 0x2e };
    this->addColor("_mymapInv51", mymapInv51);
    int mymapInv52[3] = { 0xad, 0xdc, 0x30 };
    this->addColor("_mymapInv52", mymapInv52);
    int mymapInv53[3] = { 0xaa, 0xdb, 0x32 };
    this->addColor("_mymapInv53", mymapInv53);
    int mymapInv54[3] = { 0xa7, 0xdb, 0x33 };
    this->addColor("_mymapInv54", mymapInv54);
    int mymapInv55[3] = { 0xa5, 0xda, 0x35 };
    this->addColor("_mymapInv55", mymapInv55);
    int mymapInv56[3] = { 0xa2, 0xda, 0x37 };
    this->addColor("_mymapInv56", mymapInv56);
    int mymapInv57[3] = { 0x9f, 0xd9, 0x38 };
    this->addColor("_mymapInv57", mymapInv57);
    int mymapInv58[3] = { 0x9d, 0xd9, 0x3a };
    this->addColor("_mymapInv58", mymapInv58);
    int mymapInv59[3] = { 0x9a, 0xd8, 0x3c };
    this->addColor("_mymapInv59", mymapInv59);
    int mymapInv60[3] = { 0x97, 0xd8, 0x3e };
    this->addColor("_mymapInv60", mymapInv60);
    int mymapInv61[3] = { 0x95, 0xd7, 0x3f };
    this->addColor("_mymapInv61", mymapInv61);
    int mymapInv62[3] = { 0x92, 0xd7, 0x41 };
    this->addColor("_mymapInv62", mymapInv62);
    int mymapInv63[3] = { 0x90, 0xd6, 0x43 };
    this->addColor("_mymapInv63", mymapInv63);
    int mymapInv64[3] = { 0x8d, 0xd6, 0x44 };
    this->addColor("_mymapInv64", mymapInv64);
    int mymapInv65[3] = { 0x8b, 0xd5, 0x46 };
    this->addColor("_mymapInv65", mymapInv65);
    int mymapInv66[3] = { 0x88, 0xd5, 0x47 };
    this->addColor("_mymapInv66", mymapInv66);
    int mymapInv67[3] = { 0x86, 0xd4, 0x49 };
    this->addColor("_mymapInv67", mymapInv67);
    int mymapInv68[3] = { 0x83, 0xd3, 0x4b };
    this->addColor("_mymapInv68", mymapInv68);
    int mymapInv69[3] = { 0x81, 0xd3, 0x4c };
    this->addColor("_mymapInv69", mymapInv69);
    int mymapInv70[3] = { 0x7e, 0xd2, 0x4e };
    this->addColor("_mymapInv70", mymapInv70);
    int mymapInv71[3] = { 0x7c, 0xd2, 0x4f };
    this->addColor("_mymapInv71", mymapInv71);
    int mymapInv72[3] = { 0x79, 0xd1, 0x51 };
    this->addColor("_mymapInv72", mymapInv72);
    int mymapInv73[3] = { 0x77, 0xd0, 0x52 };
    this->addColor("_mymapInv73", mymapInv73);
    int mymapInv74[3] = { 0x74, 0xd0, 0x54 };
    this->addColor("_mymapInv74", mymapInv74);
    int mymapInv75[3] = { 0x72, 0xcf, 0x55 };
    this->addColor("_mymapInv75", mymapInv75);
    int mymapInv76[3] = { 0x70, 0xce, 0x56 };
    this->addColor("_mymapInv76", mymapInv76);
    int mymapInv77[3] = { 0x6d, 0xce, 0x58 };
    this->addColor("_mymapInv77", mymapInv77);
    int mymapInv78[3] = { 0x6b, 0xcd, 0x59 };
    this->addColor("_mymapInv78", mymapInv78);
    int mymapInv79[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapInv79", mymapInv79);
    int mymapInv80[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapInv80", mymapInv80);
    int mymapInv81[3] = { 0x67, 0xcc, 0x5c };
    this->addColor("_mymapInv81", mymapInv81);
    int mymapInv82[3] = { 0x64, 0xcb, 0x5d };
    this->addColor("_mymapInv82", mymapInv82);
    int mymapInv83[3] = { 0x62, 0xca, 0x5f };
    this->addColor("_mymapInv83", mymapInv83);
    int mymapInv84[3] = { 0x60, 0xc9, 0x60 };
    this->addColor("_mymapInv84", mymapInv84);
    int mymapInv85[3] = { 0x5e, 0xc9, 0x61 };
    this->addColor("_mymapInv85", mymapInv85);
    int mymapInv86[3] = { 0x5b, 0xc8, 0x62 };
    this->addColor("_mymapInv86", mymapInv86);
    int mymapInv87[3] = { 0x59, 0xc7, 0x64 };
    this->addColor("_mymapInv87", mymapInv87);
    int mymapInv88[3] = { 0x57, 0xc6, 0x65 };
    this->addColor("_mymapInv88", mymapInv88);
    int mymapInv89[3] = { 0x55, 0xc6, 0x66 };
    this->addColor("_mymapInv89", mymapInv89);
    int mymapInv90[3] = { 0x53, 0xc5, 0x67 };
    this->addColor("_mymapInv90", mymapInv90);
    int mymapInv91[3] = { 0x51, 0xc4, 0x68 };
    this->addColor("_mymapInv91", mymapInv91);
    int mymapInv92[3] = { 0x4f, 0xc3, 0x69 };
    this->addColor("_mymapInv92", mymapInv92);
    int mymapInv93[3] = { 0x4d, 0xc2, 0x6b };
    this->addColor("_mymapInv93", mymapInv93);
    int mymapInv94[3] = { 0x4b, 0xc2, 0x6c };
    this->addColor("_mymapInv94", mymapInv94);
    int mymapInv95[3] = { 0x49, 0xc1, 0x6d };
    this->addColor("_mymapInv95", mymapInv95);
    int mymapInv96[3] = { 0x47, 0xc0, 0x6e };
    this->addColor("_mymapInv96", mymapInv96);
    int mymapInv97[3] = { 0x45, 0xbf, 0x6f };
    this->addColor("_mymapInv97", mymapInv97);
    int mymapInv98[3] = { 0x44, 0xbe, 0x70 };
    this->addColor("_mymapInv98", mymapInv98);
    int mymapInv99[3] = { 0x42, 0xbe, 0x71 };
    this->addColor("_mymapInv99", mymapInv99);
    int mymapInv100[3] = { 0x40, 0xbd, 0x72 };
    this->addColor("_mymapInv100", mymapInv100);
    int mymapInv101[3] = { 0x3e, 0xbc, 0x73 };
    this->addColor("_mymapInv101", mymapInv101);
    int mymapInv102[3] = { 0x3d, 0xbb, 0x74 };
    this->addColor("_mymapInv102", mymapInv102);
    int mymapInv103[3] = { 0x3b, 0xba, 0x75 };
    this->addColor("_mymapInv103", mymapInv103);
    int mymapInv104[3] = { 0x39, 0xb9, 0x76 };
    this->addColor("_mymapInv104", mymapInv104);
    int mymapInv105[3] = { 0x38, 0xb9, 0x76 };
    this->addColor("_mymapInv105", mymapInv105);
    int mymapInv106[3] = { 0x36, 0xb8, 0x77 };
    this->addColor("_mymapInv106", mymapInv106);
    int mymapInv107[3] = { 0x35, 0xb7, 0x78 };
    this->addColor("_mymapInv107", mymapInv107);
    int mymapInv108[3] = { 0x33, 0xb6, 0x79 };
    this->addColor("_mymapInv108", mymapInv108);
    int mymapInv109[3] = { 0x32, 0xb5, 0x7a };
    this->addColor("_mymapInv109", mymapInv109);
    int mymapInv110[3] = { 0x30, 0xb4, 0x7a };
    this->addColor("_mymapInv110", mymapInv110);
    int mymapInv111[3] = { 0x2f, 0xb3, 0x7b };
    this->addColor("_mymapInv111", mymapInv111);
    int mymapInv112[3] = { 0x2e, 0xb2, 0x7c };
    this->addColor("_mymapInv112", mymapInv112);
    int mymapInv113[3] = { 0x2c, 0xb1, 0x7d };
    this->addColor("_mymapInv113", mymapInv113);
    int mymapInv114[3] = { 0x2b, 0xb1, 0x7d };
    this->addColor("_mymapInv114", mymapInv114);
    int mymapInv115[3] = { 0x2a, 0xb0, 0x7e };
    this->addColor("_mymapInv115", mymapInv115);
    int mymapInv116[3] = { 0x29, 0xaf, 0x7f };
    this->addColor("_mymapInv116", mymapInv116);
    int mymapInv117[3] = { 0x28, 0xae, 0x7f };
    this->addColor("_mymapInv117", mymapInv117);
    int mymapInv118[3] = { 0x27, 0xad, 0x80 };
    this->addColor("_mymapInv118", mymapInv118);
    int mymapInv119[3] = { 0x26, 0xac, 0x81 };
    this->addColor("_mymapInv119", mymapInv119);
    int mymapInv120[3] = { 0x25, 0xab, 0x81 };
    this->addColor("_mymapInv120", mymapInv120);
    int mymapInv121[3] = { 0x24, 0xaa, 0x82 };
    this->addColor("_mymapInv121", mymapInv121);
    int mymapInv122[3] = { 0x23, 0xa9, 0x82 };
    this->addColor("_mymapInv122", mymapInv122);
    int mymapInv123[3] = { 0x23, 0xa8, 0x83 };
    this->addColor("_mymapInv123", mymapInv123);
    int mymapInv124[3] = { 0x22, 0xa7, 0x84 };
    this->addColor("_mymapInv124", mymapInv124);
    int mymapInv125[3] = { 0x21, 0xa7, 0x84 };
    this->addColor("_mymapInv125", mymapInv125);
    int mymapInv126[3] = { 0x21, 0xa6, 0x85 };
    this->addColor("_mymapInv126", mymapInv126);
    int mymapInv127[3] = { 0x20, 0xa5, 0x85 };
    this->addColor("_mymapInv127", mymapInv127);
    int mymapInv128[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapInv128", mymapInv128);
    int mymapInv129[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapInv129", mymapInv129);
    int mymapInv130[3] = { 0x1f, 0xa3, 0x86 };
    this->addColor("_mymapInv130", mymapInv130);
    int mymapInv131[3] = { 0x1f, 0xa2, 0x86 };
    this->addColor("_mymapInv131", mymapInv131);
    int mymapInv132[3] = { 0x1f, 0xa1, 0x87 };
    this->addColor("_mymapInv132", mymapInv132);
    int mymapInv133[3] = { 0x1e, 0xa0, 0x87 };
    this->addColor("_mymapInv133", mymapInv133);
    int mymapInv134[3] = { 0x1e, 0x9f, 0x88 };
    this->addColor("_mymapInv134", mymapInv134);
    int mymapInv135[3] = { 0x1e, 0x9e, 0x88 };
    this->addColor("_mymapInv135", mymapInv135);
    int mymapInv136[3] = { 0x1e, 0x9d, 0x88 };
    this->addColor("_mymapInv136", mymapInv136);
    int mymapInv137[3] = { 0x1e, 0x9c, 0x89 };
    this->addColor("_mymapInv137", mymapInv137);
    int mymapInv138[3] = { 0x1e, 0x9b, 0x89 };
    this->addColor("_mymapInv138", mymapInv138);
    int mymapInv139[3] = { 0x1e, 0x9a, 0x89 };
    this->addColor("_mymapInv139", mymapInv139);
    int mymapInv140[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapInv140", mymapInv140);
    int mymapInv141[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapInv141", mymapInv141);
    int mymapInv142[3] = { 0x1e, 0x98, 0x8a };
    this->addColor("_mymapInv142", mymapInv142);
    int mymapInv143[3] = { 0x1e, 0x97, 0x8a };
    this->addColor("_mymapInv143", mymapInv143);
    int mymapInv144[3] = { 0x1f, 0x96, 0x8b };
    this->addColor("_mymapInv144", mymapInv144);
    int mymapInv145[3] = { 0x1f, 0x95, 0x8b };
    this->addColor("_mymapInv145", mymapInv145);
    int mymapInv146[3] = { 0x1f, 0x94, 0x8b };
    this->addColor("_mymapInv146", mymapInv146);
    int mymapInv147[3] = { 0x1f, 0x93, 0x8b };
    this->addColor("_mymapInv147", mymapInv147);
    int mymapInv148[3] = { 0x1f, 0x92, 0x8c };
    this->addColor("_mymapInv148", mymapInv148);
    int mymapInv149[3] = { 0x20, 0x91, 0x8c };
    this->addColor("_mymapInv149", mymapInv149);
    int mymapInv150[3] = { 0x20, 0x90, 0x8c };
    this->addColor("_mymapInv150", mymapInv150);
    int mymapInv151[3] = { 0x20, 0x8f, 0x8c };
    this->addColor("_mymapInv151", mymapInv151);
    int mymapInv152[3] = { 0x21, 0x8e, 0x8c };
    this->addColor("_mymapInv152", mymapInv152);
    int mymapInv153[3] = { 0x21, 0x8d, 0x8c };
    this->addColor("_mymapInv153", mymapInv153);
    int mymapInv154[3] = { 0x21, 0x8c, 0x8d };
    this->addColor("_mymapInv154", mymapInv154);
    int mymapInv155[3] = { 0x22, 0x8b, 0x8d };
    this->addColor("_mymapInv155", mymapInv155);
    int mymapInv156[3] = { 0x22, 0x8a, 0x8d };
    this->addColor("_mymapInv156", mymapInv156);
    int mymapInv157[3] = { 0x22, 0x89, 0x8d };
    this->addColor("_mymapInv157", mymapInv157);
    int mymapInv158[3] = { 0x23, 0x89, 0x8d };
    this->addColor("_mymapInv158", mymapInv158);
    int mymapInv159[3] = { 0x23, 0x88, 0x8d };
    this->addColor("_mymapInv159", mymapInv159);
    int mymapInv160[3] = { 0x23, 0x87, 0x8d };
    this->addColor("_mymapInv160", mymapInv160);
    int mymapInv161[3] = { 0x24, 0x86, 0x8d };
    this->addColor("_mymapInv161", mymapInv161);
    int mymapInv162[3] = { 0x24, 0x85, 0x8d };
    this->addColor("_mymapInv162", mymapInv162);
    int mymapInv163[3] = { 0x24, 0x84, 0x8d };
    this->addColor("_mymapInv163", mymapInv163);
    int mymapInv164[3] = { 0x25, 0x83, 0x8d };
    this->addColor("_mymapInv164", mymapInv164);
    int mymapInv165[3] = { 0x25, 0x82, 0x8e };
    this->addColor("_mymapInv165", mymapInv165);
    int mymapInv166[3] = { 0x26, 0x81, 0x8e };
    this->addColor("_mymapInv166", mymapInv166);
    int mymapInv167[3] = { 0x26, 0x80, 0x8e };
    this->addColor("_mymapInv167", mymapInv167);
    int mymapInv168[3] = { 0x26, 0x7f, 0x8e };
    this->addColor("_mymapInv168", mymapInv168);
    int mymapInv169[3] = { 0x27, 0x7e, 0x8e };
    this->addColor("_mymapInv169", mymapInv169);
    int mymapInv170[3] = { 0x27, 0x7d, 0x8e };
    this->addColor("_mymapInv170", mymapInv170);
    int mymapInv171[3] = { 0x27, 0x7c, 0x8e };
    this->addColor("_mymapInv171", mymapInv171);
    int mymapInv172[3] = { 0x28, 0x7b, 0x8e };
    this->addColor("_mymapInv172", mymapInv172);
    int mymapInv173[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapInv173", mymapInv173);
    int mymapInv174[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapInv174", mymapInv174);
    int mymapInv175[3] = { 0x29, 0x79, 0x8e };
    this->addColor("_mymapInv175", mymapInv175);
    int mymapInv176[3] = { 0x29, 0x78, 0x8e };
    this->addColor("_mymapInv176", mymapInv176);
    int mymapInv177[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapInv177", mymapInv177);
    int mymapInv178[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapInv178", mymapInv178);
    int mymapInv179[3] = { 0x2a, 0x76, 0x8e };
    this->addColor("_mymapInv179", mymapInv179);
    int mymapInv180[3] = { 0x2a, 0x75, 0x8e };
    this->addColor("_mymapInv180", mymapInv180);
    int mymapInv181[3] = { 0x2b, 0x74, 0x8e };
    this->addColor("_mymapInv181", mymapInv181);
    int mymapInv182[3] = { 0x2b, 0x73, 0x8e };
    this->addColor("_mymapInv182", mymapInv182);
    int mymapInv183[3] = { 0x2c, 0x72, 0x8e };
    this->addColor("_mymapInv183", mymapInv183);
    int mymapInv184[3] = { 0x2c, 0x71, 0x8e };
    this->addColor("_mymapInv184", mymapInv184);
    int mymapInv185[3] = { 0x2c, 0x70, 0x8e };
    this->addColor("_mymapInv185", mymapInv185);
    int mymapInv186[3] = { 0x2d, 0x6f, 0x8e };
    this->addColor("_mymapInv186", mymapInv186);
    int mymapInv187[3] = { 0x2d, 0x6e, 0x8e };
    this->addColor("_mymapInv187", mymapInv187);
    int mymapInv188[3] = { 0x2e, 0x6d, 0x8e };
    this->addColor("_mymapInv188", mymapInv188);
    int mymapInv189[3] = { 0x2e, 0x6c, 0x8e };
    this->addColor("_mymapInv189", mymapInv189);
    int mymapInv190[3] = { 0x2e, 0x6b, 0x8e };
    this->addColor("_mymapInv190", mymapInv190);
    int mymapInv191[3] = { 0x2f, 0x6a, 0x8d };
    this->addColor("_mymapInv191", mymapInv191);
    int mymapInv192[3] = { 0x2f, 0x69, 0x8d };
    this->addColor("_mymapInv192", mymapInv192);
    int mymapInv193[3] = { 0x30, 0x68, 0x8d };
    this->addColor("_mymapInv193", mymapInv193);
    int mymapInv194[3] = { 0x30, 0x67, 0x8d };
    this->addColor("_mymapInv194", mymapInv194);
    int mymapInv195[3] = { 0x31, 0x66, 0x8d };
    this->addColor("_mymapInv195", mymapInv195);
    int mymapInv196[3] = { 0x31, 0x65, 0x8d };
    this->addColor("_mymapInv196", mymapInv196);
    int mymapInv197[3] = { 0x31, 0x64, 0x8d };
    this->addColor("_mymapInv197", mymapInv197);
    int mymapInv198[3] = { 0x32, 0x63, 0x8d };
    this->addColor("_mymapInv198", mymapInv198);
    int mymapInv199[3] = { 0x32, 0x62, 0x8d };
    this->addColor("_mymapInv199", mymapInv199);
    int mymapInv200[3] = { 0x33, 0x61, 0x8d };
    this->addColor("_mymapInv200", mymapInv200);
    int mymapInv201[3] = { 0x33, 0x60, 0x8d };
    this->addColor("_mymapInv201", mymapInv201);
    int mymapInv202[3] = { 0x34, 0x5f, 0x8d };
    this->addColor("_mymapInv202", mymapInv202);
    int mymapInv203[3] = { 0x34, 0x5e, 0x8d };
    this->addColor("_mymapInv203", mymapInv203);
    int mymapInv204[3] = { 0x35, 0x5d, 0x8c };
    this->addColor("_mymapInv204", mymapInv204);
    int mymapInv205[3] = { 0x35, 0x5c, 0x8c };
    this->addColor("_mymapInv205", mymapInv205);
    int mymapInv206[3] = { 0x36, 0x5b, 0x8c };
    this->addColor("_mymapInv206", mymapInv206);
    int mymapInv207[3] = { 0x36, 0x5a, 0x8c };
    this->addColor("_mymapInv207", mymapInv207);
    int mymapInv208[3] = { 0x37, 0x59, 0x8c };
    this->addColor("_mymapInv208", mymapInv208);
    int mymapInv209[3] = { 0x37, 0x58, 0x8c };
    this->addColor("_mymapInv209", mymapInv209);
    int mymapInv210[3] = { 0x38, 0x57, 0x8c };
    this->addColor("_mymapInv210", mymapInv210);
    int mymapInv211[3] = { 0x38, 0x56, 0x8b };
    this->addColor("_mymapInv211", mymapInv211);
    int mymapInv212[3] = { 0x39, 0x55, 0x8b };
    this->addColor("_mymapInv212", mymapInv212);
    int mymapInv213[3] = { 0x39, 0x54, 0x8b };
    this->addColor("_mymapInv213", mymapInv213);
    int mymapInv214[3] = { 0x3a, 0x53, 0x8b };
    this->addColor("_mymapInv214", mymapInv214);
    int mymapInv215[3] = { 0x3a, 0x52, 0x8b };
    this->addColor("_mymapInv215", mymapInv215);
    int mymapInv216[3] = { 0x3b, 0x51, 0x8a };
    this->addColor("_mymapInv216", mymapInv216);
    int mymapInv217[3] = { 0x3b, 0x50, 0x8a };
    this->addColor("_mymapInv217", mymapInv217);
    int mymapInv218[3] = { 0x3c, 0x4e, 0x8a };
    this->addColor("_mymapInv218", mymapInv218);
    int mymapInv219[3] = { 0x3c, 0x4d, 0x8a };
    this->addColor("_mymapInv219", mymapInv219);
    int mymapInv220[3] = { 0x3d, 0x4c, 0x89 };
    this->addColor("_mymapInv220", mymapInv220);
    int mymapInv221[3] = { 0x3d, 0x4b, 0x89 };
    this->addColor("_mymapInv221", mymapInv221);
    int mymapInv222[3] = { 0x3d, 0x4a, 0x89 };
    this->addColor("_mymapInv222", mymapInv222);
    int mymapInv223[3] = { 0x3e, 0x49, 0x89 };
    this->addColor("_mymapInv223", mymapInv223);
    int mymapInv224[3] = { 0x3e, 0x48, 0x88 };
    this->addColor("_mymapInv224", mymapInv224);
    int mymapInv225[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapInv225", mymapInv225);
    int mymapInv226[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapInv226", mymapInv226);
    int mymapInv227[3] = { 0x3f, 0x45, 0x87 };
    this->addColor("_mymapInv227", mymapInv227);
    int mymapInv228[3] = { 0x40, 0x44, 0x87 };
    this->addColor("_mymapInv228", mymapInv228);
    int mymapInv229[3] = { 0x40, 0x43, 0x87 };
    this->addColor("_mymapInv229", mymapInv229);
    int mymapInv230[3] = { 0x41, 0x42, 0x86 };
    this->addColor("_mymapInv230", mymapInv230);
    int mymapInv231[3] = { 0x41, 0x41, 0x86 };
    this->addColor("_mymapInv231", mymapInv231);
    int mymapInv232[3] = { 0x42, 0x40, 0x85 };
    this->addColor("_mymapInv232", mymapInv232);
    int mymapInv233[3] = { 0x42, 0x3e, 0x85 };
    this->addColor("_mymapInv233", mymapInv233);
    int mymapInv234[3] = { 0x42, 0x3d, 0x84 };
    this->addColor("_mymapInv234", mymapInv234);
    int mymapInv235[3] = { 0x43, 0x3c, 0x84 };
    this->addColor("_mymapInv235", mymapInv235);
    int mymapInv236[3] = { 0x43, 0x3b, 0x83 };
    this->addColor("_mymapInv236", mymapInv236);
    int mymapInv237[3] = { 0x43, 0x3a, 0x83 };
    this->addColor("_mymapInv237", mymapInv237);
    int mymapInv238[3] = { 0x44, 0x39, 0x82 };
    this->addColor("_mymapInv238", mymapInv238);
    int mymapInv239[3] = { 0x44, 0x37, 0x81 };
    this->addColor("_mymapInv239", mymapInv239);
    int mymapInv240[3] = { 0x45, 0x36, 0x81 };
    this->addColor("_mymapInv240", mymapInv240);
    int mymapInv241[3] = { 0x45, 0x35, 0x80 };
    this->addColor("_mymapInv241", mymapInv241);
    int mymapInv242[3] = { 0x45, 0x34, 0x7f };
    this->addColor("_mymapInv242", mymapInv242);
    int mymapInv243[3] = { 0x45, 0x32, 0x7f };
    this->addColor("_mymapInv243", mymapInv243);
    int mymapInv244[3] = { 0x46, 0x31, 0x7e };
    this->addColor("_mymapInv244", mymapInv244);
    int mymapInv245[3] = { 0x46, 0x30, 0x7d };
    this->addColor("_mymapInv245", mymapInv245);
    int mymapInv246[3] = { 0x46, 0x2f, 0x7c };
    this->addColor("_mymapInv246", mymapInv246);
    int mymapInv247[3] = { 0x46, 0x2d, 0x7c };
    this->addColor("_mymapInv247", mymapInv247);
    int mymapInv248[3] = { 0x47, 0x2c, 0x7b };
    this->addColor("_mymapInv248", mymapInv248);
    int mymapInv249[3] = { 0x47, 0x2b, 0x7a };
    this->addColor("_mymapInv249", mymapInv249);
    int mymapInv250[3] = { 0x47, 0x2a, 0x79 };
    this->addColor("_mymapInv250", mymapInv250);
    int mymapInv251[3] = { 0x47, 0x28, 0x78 };
    this->addColor("_mymapInv251", mymapInv251);
    int mymapInv252[3] = { 0x47, 0x27, 0x77 };
    this->addColor("_mymapInv252", mymapInv252);
    int mymapInv253[3] = { 0x47, 0x26, 0x76 };
    this->addColor("_mymapInv253", mymapInv253);
    int mymapInv254[3] = { 0x47, 0x25, 0x75 };
    this->addColor("_mymapInv254", mymapInv254);
    int mymapInv255[3] = { 0x48, 0x23, 0x74 };
    this->addColor("_mymapInv255", mymapInv255);
    if (this->getPaletteByName("margulies_inv") == NULL) {
        Palette mymapInv;
        mymapInv.setName("margulies_inv");
        mymapInv.addScalarAndColor( 1.000000f, "_mymapInv0");
        mymapInv.addScalarAndColor( 0.992157f, "_mymapInv1");
        mymapInv.addScalarAndColor( 0.984314f, "_mymapInv2");
        mymapInv.addScalarAndColor( 0.976471f, "_mymapInv3");
        mymapInv.addScalarAndColor( 0.968627f, "_mymapInv4");
        mymapInv.addScalarAndColor( 0.960784f, "_mymapInv5");
        mymapInv.addScalarAndColor( 0.952941f, "_mymapInv6");
        mymapInv.addScalarAndColor( 0.945098f, "_mymapInv7");
        mymapInv.addScalarAndColor( 0.937255f, "_mymapInv8");
        mymapInv.addScalarAndColor( 0.929412f, "_mymapInv9");
        mymapInv.addScalarAndColor( 0.921569f, "_mymapInv10");
        mymapInv.addScalarAndColor( 0.913725f, "_mymapInv11");
        mymapInv.addScalarAndColor( 0.905882f, "_mymapInv12");
        mymapInv.addScalarAndColor( 0.898039f, "_mymapInv13");
        mymapInv.addScalarAndColor( 0.890196f, "_mymapInv14");
        mymapInv.addScalarAndColor( 0.882353f, "_mymapInv15");
        mymapInv.addScalarAndColor( 0.874510f, "_mymapInv16");
        mymapInv.addScalarAndColor( 0.866667f, "_mymapInv17");
        mymapInv.addScalarAndColor( 0.858824f, "_mymapInv18");
        mymapInv.addScalarAndColor( 0.850980f, "_mymapInv19");
        mymapInv.addScalarAndColor( 0.843137f, "_mymapInv20");
        mymapInv.addScalarAndColor( 0.835294f, "_mymapInv21");
        mymapInv.addScalarAndColor( 0.827451f, "_mymapInv22");
        mymapInv.addScalarAndColor( 0.819608f, "_mymapInv23");
        mymapInv.addScalarAndColor( 0.811765f, "_mymapInv24");
        mymapInv.addScalarAndColor( 0.803922f, "_mymapInv25");
        mymapInv.addScalarAndColor( 0.796078f, "_mymapInv26");
        mymapInv.addScalarAndColor( 0.788235f, "_mymapInv27");
        mymapInv.addScalarAndColor( 0.780392f, "_mymapInv28");
        mymapInv.addScalarAndColor( 0.772549f, "_mymapInv29");
        mymapInv.addScalarAndColor( 0.764706f, "_mymapInv30");
        mymapInv.addScalarAndColor( 0.756863f, "_mymapInv31");
        mymapInv.addScalarAndColor( 0.749020f, "_mymapInv32");
        mymapInv.addScalarAndColor( 0.741176f, "_mymapInv33");
        mymapInv.addScalarAndColor( 0.733333f, "_mymapInv34");
        mymapInv.addScalarAndColor( 0.725490f, "_mymapInv35");
        mymapInv.addScalarAndColor( 0.717647f, "_mymapInv36");
        mymapInv.addScalarAndColor( 0.709804f, "_mymapInv37");
        mymapInv.addScalarAndColor( 0.701961f, "_mymapInv38");
        mymapInv.addScalarAndColor( 0.694118f, "_mymapInv39");
        mymapInv.addScalarAndColor( 0.686275f, "_mymapInv40");
        mymapInv.addScalarAndColor( 0.678431f, "_mymapInv41");
        mymapInv.addScalarAndColor( 0.670588f, "_mymapInv42");
        mymapInv.addScalarAndColor( 0.662745f, "_mymapInv43");
        mymapInv.addScalarAndColor( 0.654902f, "_mymapInv44");
        mymapInv.addScalarAndColor( 0.647059f, "_mymapInv45");
        mymapInv.addScalarAndColor( 0.639216f, "_mymapInv46");
        mymapInv.addScalarAndColor( 0.631373f, "_mymapInv47");
        mymapInv.addScalarAndColor( 0.623529f, "_mymapInv48");
        mymapInv.addScalarAndColor( 0.615686f, "_mymapInv49");
        mymapInv.addScalarAndColor( 0.607843f, "_mymapInv50");
        mymapInv.addScalarAndColor( 0.600000f, "_mymapInv51");
        mymapInv.addScalarAndColor( 0.592157f, "_mymapInv52");
        mymapInv.addScalarAndColor( 0.584314f, "_mymapInv53");
        mymapInv.addScalarAndColor( 0.576471f, "_mymapInv54");
        mymapInv.addScalarAndColor( 0.568627f, "_mymapInv55");
        mymapInv.addScalarAndColor( 0.560784f, "_mymapInv56");
        mymapInv.addScalarAndColor( 0.552941f, "_mymapInv57");
        mymapInv.addScalarAndColor( 0.545098f, "_mymapInv58");
        mymapInv.addScalarAndColor( 0.537255f, "_mymapInv59");
        mymapInv.addScalarAndColor( 0.529412f, "_mymapInv60");
        mymapInv.addScalarAndColor( 0.521569f, "_mymapInv61");
        mymapInv.addScalarAndColor( 0.513725f, "_mymapInv62");
        mymapInv.addScalarAndColor( 0.505882f, "_mymapInv63");
        mymapInv.addScalarAndColor( 0.498039f, "_mymapInv64");
        mymapInv.addScalarAndColor( 0.490196f, "_mymapInv65");
        mymapInv.addScalarAndColor( 0.482353f, "_mymapInv66");
        mymapInv.addScalarAndColor( 0.474510f, "_mymapInv67");
        mymapInv.addScalarAndColor( 0.466667f, "_mymapInv68");
        mymapInv.addScalarAndColor( 0.458824f, "_mymapInv69");
        mymapInv.addScalarAndColor( 0.450980f, "_mymapInv70");
        mymapInv.addScalarAndColor( 0.443137f, "_mymapInv71");
        mymapInv.addScalarAndColor( 0.435294f, "_mymapInv72");
        mymapInv.addScalarAndColor( 0.427451f, "_mymapInv73");
        mymapInv.addScalarAndColor( 0.419608f, "_mymapInv74");
        mymapInv.addScalarAndColor( 0.411765f, "_mymapInv75");
        mymapInv.addScalarAndColor( 0.403922f, "_mymapInv76");
        mymapInv.addScalarAndColor( 0.396078f, "_mymapInv77");
        mymapInv.addScalarAndColor( 0.388235f, "_mymapInv78");
        mymapInv.addScalarAndColor( 0.380392f, "_mymapInv79");
        mymapInv.addScalarAndColor( 0.372549f, "_mymapInv80");
        mymapInv.addScalarAndColor( 0.364706f, "_mymapInv81");
        mymapInv.addScalarAndColor( 0.356863f, "_mymapInv82");
        mymapInv.addScalarAndColor( 0.349020f, "_mymapInv83");
        mymapInv.addScalarAndColor( 0.341176f, "_mymapInv84");
        mymapInv.addScalarAndColor( 0.333333f, "_mymapInv85");
        mymapInv.addScalarAndColor( 0.325490f, "_mymapInv86");
        mymapInv.addScalarAndColor( 0.317647f, "_mymapInv87");
        mymapInv.addScalarAndColor( 0.309804f, "_mymapInv88");
        mymapInv.addScalarAndColor( 0.301961f, "_mymapInv89");
        mymapInv.addScalarAndColor( 0.294118f, "_mymapInv90");
        mymapInv.addScalarAndColor( 0.286275f, "_mymapInv91");
        mymapInv.addScalarAndColor( 0.278431f, "_mymapInv92");
        mymapInv.addScalarAndColor( 0.270588f, "_mymapInv93");
        mymapInv.addScalarAndColor( 0.262745f, "_mymapInv94");
        mymapInv.addScalarAndColor( 0.254902f, "_mymapInv95");
        mymapInv.addScalarAndColor( 0.247059f, "_mymapInv96");
        mymapInv.addScalarAndColor( 0.239216f, "_mymapInv97");
        mymapInv.addScalarAndColor( 0.231373f, "_mymapInv98");
        mymapInv.addScalarAndColor( 0.223529f, "_mymapInv99");
        mymapInv.addScalarAndColor( 0.215686f, "_mymapInv100");
        mymapInv.addScalarAndColor( 0.207843f, "_mymapInv101");
        mymapInv.addScalarAndColor( 0.200000f, "_mymapInv102");
        mymapInv.addScalarAndColor( 0.192157f, "_mymapInv103");
        mymapInv.addScalarAndColor( 0.184314f, "_mymapInv104");
        mymapInv.addScalarAndColor( 0.176471f, "_mymapInv105");
        mymapInv.addScalarAndColor( 0.168627f, "_mymapInv106");
        mymapInv.addScalarAndColor( 0.160784f, "_mymapInv107");
        mymapInv.addScalarAndColor( 0.152941f, "_mymapInv108");
        mymapInv.addScalarAndColor( 0.145098f, "_mymapInv109");
        mymapInv.addScalarAndColor( 0.137255f, "_mymapInv110");
        mymapInv.addScalarAndColor( 0.129412f, "_mymapInv111");
        mymapInv.addScalarAndColor( 0.121569f, "_mymapInv112");
        mymapInv.addScalarAndColor( 0.113725f, "_mymapInv113");
        mymapInv.addScalarAndColor( 0.105882f, "_mymapInv114");
        mymapInv.addScalarAndColor( 0.098039f, "_mymapInv115");
        mymapInv.addScalarAndColor( 0.090196f, "_mymapInv116");
        mymapInv.addScalarAndColor( 0.082353f, "_mymapInv117");
        mymapInv.addScalarAndColor( 0.074510f, "_mymapInv118");
        mymapInv.addScalarAndColor( 0.066667f, "_mymapInv119");
        mymapInv.addScalarAndColor( 0.058824f, "_mymapInv120");
        mymapInv.addScalarAndColor( 0.050980f, "_mymapInv121");
        mymapInv.addScalarAndColor( 0.043137f, "_mymapInv122");
        mymapInv.addScalarAndColor( 0.035294f, "_mymapInv123");
        mymapInv.addScalarAndColor( 0.027451f, "_mymapInv124");
        mymapInv.addScalarAndColor( 0.019608f, "_mymapInv125");
        mymapInv.addScalarAndColor( 0.011765f, "_mymapInv126");
        mymapInv.addScalarAndColor( 0.003922f, "_mymapInv127");
        mymapInv.addScalarAndColor( -0.003922f, "_mymapInv128");
        mymapInv.addScalarAndColor( -0.011765f, "_mymapInv129");
        mymapInv.addScalarAndColor( -0.019608f, "_mymapInv130");
        mymapInv.addScalarAndColor( -0.027451f, "_mymapInv131");
        mymapInv.addScalarAndColor( -0.035294f, "_mymapInv132");
        mymapInv.addScalarAndColor( -0.043137f, "_mymapInv133");
        mymapInv.addScalarAndColor( -0.050980f, "_mymapInv134");
        mymapInv.addScalarAndColor( -0.058824f, "_mymapInv135");
        mymapInv.addScalarAndColor( -0.066667f, "_mymapInv136");
        mymapInv.addScalarAndColor( -0.074510f, "_mymapInv137");
        mymapInv.addScalarAndColor( -0.082353f, "_mymapInv138");
        mymapInv.addScalarAndColor( -0.090196f, "_mymapInv139");
        mymapInv.addScalarAndColor( -0.098039f, "_mymapInv140");
        mymapInv.addScalarAndColor( -0.105882f, "_mymapInv141");
        mymapInv.addScalarAndColor( -0.113725f, "_mymapInv142");
        mymapInv.addScalarAndColor( -0.121569f, "_mymapInv143");
        mymapInv.addScalarAndColor( -0.129412f, "_mymapInv144");
        mymapInv.addScalarAndColor( -0.137255f, "_mymapInv145");
        mymapInv.addScalarAndColor( -0.145098f, "_mymapInv146");
        mymapInv.addScalarAndColor( -0.152941f, "_mymapInv147");
        mymapInv.addScalarAndColor( -0.160784f, "_mymapInv148");
        mymapInv.addScalarAndColor( -0.168627f, "_mymapInv149");
        mymapInv.addScalarAndColor( -0.176471f, "_mymapInv150");
        mymapInv.addScalarAndColor( -0.184314f, "_mymapInv151");
        mymapInv.addScalarAndColor( -0.192157f, "_mymapInv152");
        mymapInv.addScalarAndColor( -0.200000f, "_mymapInv153");
        mymapInv.addScalarAndColor( -0.207843f, "_mymapInv154");
        mymapInv.addScalarAndColor( -0.215686f, "_mymapInv155");
        mymapInv.addScalarAndColor( -0.223529f, "_mymapInv156");
        mymapInv.addScalarAndColor( -0.231373f, "_mymapInv157");
        mymapInv.addScalarAndColor( -0.239216f, "_mymapInv158");
        mymapInv.addScalarAndColor( -0.247059f, "_mymapInv159");
        mymapInv.addScalarAndColor( -0.254902f, "_mymapInv160");
        mymapInv.addScalarAndColor( -0.262745f, "_mymapInv161");
        mymapInv.addScalarAndColor( -0.270588f, "_mymapInv162");
        mymapInv.addScalarAndColor( -0.278431f, "_mymapInv163");
        mymapInv.addScalarAndColor( -0.286275f, "_mymapInv164");
        mymapInv.addScalarAndColor( -0.294118f, "_mymapInv165");
        mymapInv.addScalarAndColor( -0.301961f, "_mymapInv166");
        mymapInv.addScalarAndColor( -0.309804f, "_mymapInv167");
        mymapInv.addScalarAndColor( -0.317647f, "_mymapInv168");
        mymapInv.addScalarAndColor( -0.325490f, "_mymapInv169");
        mymapInv.addScalarAndColor( -0.333333f, "_mymapInv170");
        mymapInv.addScalarAndColor( -0.341176f, "_mymapInv171");
        mymapInv.addScalarAndColor( -0.349020f, "_mymapInv172");
        mymapInv.addScalarAndColor( -0.356863f, "_mymapInv173");
        mymapInv.addScalarAndColor( -0.364706f, "_mymapInv174");
        mymapInv.addScalarAndColor( -0.372549f, "_mymapInv175");
        mymapInv.addScalarAndColor( -0.380392f, "_mymapInv176");
        mymapInv.addScalarAndColor( -0.388235f, "_mymapInv177");
        mymapInv.addScalarAndColor( -0.396078f, "_mymapInv178");
        mymapInv.addScalarAndColor( -0.403922f, "_mymapInv179");
        mymapInv.addScalarAndColor( -0.411765f, "_mymapInv180");
        mymapInv.addScalarAndColor( -0.419608f, "_mymapInv181");
        mymapInv.addScalarAndColor( -0.427451f, "_mymapInv182");
        mymapInv.addScalarAndColor( -0.435294f, "_mymapInv183");
        mymapInv.addScalarAndColor( -0.443137f, "_mymapInv184");
        mymapInv.addScalarAndColor( -0.450980f, "_mymapInv185");
        mymapInv.addScalarAndColor( -0.458824f, "_mymapInv186");
        mymapInv.addScalarAndColor( -0.466667f, "_mymapInv187");
        mymapInv.addScalarAndColor( -0.474510f, "_mymapInv188");
        mymapInv.addScalarAndColor( -0.482353f, "_mymapInv189");
        mymapInv.addScalarAndColor( -0.490196f, "_mymapInv190");
        mymapInv.addScalarAndColor( -0.498039f, "_mymapInv191");
        mymapInv.addScalarAndColor( -0.505882f, "_mymapInv192");
        mymapInv.addScalarAndColor( -0.513725f, "_mymapInv193");
        mymapInv.addScalarAndColor( -0.521569f, "_mymapInv194");
        mymapInv.addScalarAndColor( -0.529412f, "_mymapInv195");
        mymapInv.addScalarAndColor( -0.537255f, "_mymapInv196");
        mymapInv.addScalarAndColor( -0.545098f, "_mymapInv197");
        mymapInv.addScalarAndColor( -0.552941f, "_mymapInv198");
        mymapInv.addScalarAndColor( -0.560784f, "_mymapInv199");
        mymapInv.addScalarAndColor( -0.568627f, "_mymapInv200");
        mymapInv.addScalarAndColor( -0.576471f, "_mymapInv201");
        mymapInv.addScalarAndColor( -0.584314f, "_mymapInv202");
        mymapInv.addScalarAndColor( -0.592157f, "_mymapInv203");
        mymapInv.addScalarAndColor( -0.600000f, "_mymapInv204");
        mymapInv.addScalarAndColor( -0.607843f, "_mymapInv205");
        mymapInv.addScalarAndColor( -0.615686f, "_mymapInv206");
        mymapInv.addScalarAndColor( -0.623529f, "_mymapInv207");
        mymapInv.addScalarAndColor( -0.631373f, "_mymapInv208");
        mymapInv.addScalarAndColor( -0.639216f, "_mymapInv209");
        mymapInv.addScalarAndColor( -0.647059f, "_mymapInv210");
        mymapInv.addScalarAndColor( -0.654902f, "_mymapInv211");
        mymapInv.addScalarAndColor( -0.662745f, "_mymapInv212");
        mymapInv.addScalarAndColor( -0.670588f, "_mymapInv213");
        mymapInv.addScalarAndColor( -0.678431f, "_mymapInv214");
        mymapInv.addScalarAndColor( -0.686275f, "_mymapInv215");
        mymapInv.addScalarAndColor( -0.694118f, "_mymapInv216");
        mymapInv.addScalarAndColor( -0.701961f, "_mymapInv217");
        mymapInv.addScalarAndColor( -0.709804f, "_mymapInv218");
        mymapInv.addScalarAndColor( -0.717647f, "_mymapInv219");
        mymapInv.addScalarAndColor( -0.725490f, "_mymapInv220");
        mymapInv.addScalarAndColor( -0.733333f, "_mymapInv221");
        mymapInv.addScalarAndColor( -0.741176f, "_mymapInv222");
        mymapInv.addScalarAndColor( -0.749020f, "_mymapInv223");
        mymapInv.addScalarAndColor( -0.756863f, "_mymapInv224");
        mymapInv.addScalarAndColor( -0.764706f, "_mymapInv225");
        mymapInv.addScalarAndColor( -0.772549f, "_mymapInv226");
        mymapInv.addScalarAndColor( -0.780392f, "_mymapInv227");
        mymapInv.addScalarAndColor( -0.788235f, "_mymapInv228");
        mymapInv.addScalarAndColor( -0.796078f, "_mymapInv229");
        mymapInv.addScalarAndColor( -0.803922f, "_mymapInv230");
        mymapInv.addScalarAndColor( -0.811765f, "_mymapInv231");
        mymapInv.addScalarAndColor( -0.819608f, "_mymapInv232");
        mymapInv.addScalarAndColor( -0.827451f, "_mymapInv233");
        mymapInv.addScalarAndColor( -0.835294f, "_mymapInv234");
        mymapInv.addScalarAndColor( -0.843137f, "_mymapInv235");
        mymapInv.addScalarAndColor( -0.850980f, "_mymapInv236");
        mymapInv.addScalarAndColor( -0.858824f, "_mymapInv237");
        mymapInv.addScalarAndColor( -0.866667f, "_mymapInv238");
        mymapInv.addScalarAndColor( -0.874510f, "_mymapInv239");
        mymapInv.addScalarAndColor( -0.882353f, "_mymapInv240");
        mymapInv.addScalarAndColor( -0.890196f, "_mymapInv241");
        mymapInv.addScalarAndColor( -0.898039f, "_mymapInv242");
        mymapInv.addScalarAndColor( -0.905882f, "_mymapInv243");
        mymapInv.addScalarAndColor( -0.913725f, "_mymapInv244");
        mymapInv.addScalarAndColor( -0.921569f, "_mymapInv245");
        mymapInv.addScalarAndColor( -0.929412f, "_mymapInv246");
        mymapInv.addScalarAndColor( -0.937255f, "_mymapInv247");
        mymapInv.addScalarAndColor( -0.945098f, "_mymapInv248");
        mymapInv.addScalarAndColor( -0.952941f, "_mymapInv249");
        mymapInv.addScalarAndColor( -0.960784f, "_mymapInv250");
        mymapInv.addScalarAndColor( -0.968627f, "_mymapInv251");
        mymapInv.addScalarAndColor( -0.976471f, "_mymapInv252");
        mymapInv.addScalarAndColor( -0.984314f, "_mymapInv253");
        mymapInv.addScalarAndColor( -0.992157f, "_mymapInv254");
        mymapInv.addScalarAndColor( -1.000000f, "_mymapInv255");
        addPalette(mymapInv);
    }
    int mymapInvPos0[3] = { 0x66, 0x25, 0x6 };
    this->addColor("_mymapInvPos0", mymapInvPos0);
    int mymapInvPos1[3] = { 0x71, 0x28, 0x5 };
    this->addColor("_mymapInvPos1", mymapInvPos1);
    int mymapInvPos2[3] = { 0x7e, 0x2c, 0x5 };
    this->addColor("_mymapInvPos2", mymapInvPos2);
    int mymapInvPos3[3] = { 0x8a, 0x2f, 0x4 };
    this->addColor("_mymapInvPos3", mymapInvPos3);
    int mymapInvPos4[3] = { 0x97, 0x33, 0x4 };
    this->addColor("_mymapInvPos4", mymapInvPos4);
    int mymapInvPos5[3] = { 0xa4, 0x39, 0x3 };
    this->addColor("_mymapInvPos5", mymapInvPos5);
    int mymapInvPos6[3] = { 0xb1, 0x3f, 0x3 };
    this->addColor("_mymapInvPos6", mymapInvPos6);
    int mymapInvPos7[3] = { 0xbe, 0x45, 0x2 };
    this->addColor("_mymapInvPos7", mymapInvPos7);
    int mymapInvPos8[3] = { 0xca, 0x4b, 0x2 };
    this->addColor("_mymapInvPos8", mymapInvPos8);
    int mymapInvPos9[3] = { 0xd3, 0x54, 0x6 };
    this->addColor("_mymapInvPos9", mymapInvPos9);
    int mymapInvPos10[3] = { 0xdb, 0x5d, 0xa };
    this->addColor("_mymapInvPos10", mymapInvPos10);
    int mymapInvPos11[3] = { 0xe3, 0x66, 0xf };
    this->addColor("_mymapInvPos11", mymapInvPos11);
    int mymapInvPos12[3] = { 0xeb, 0x6f, 0x13 };
    this->addColor("_mymapInvPos12", mymapInvPos12);
    int mymapInvPos13[3] = { 0xf0, 0x79, 0x18 };
    this->addColor("_mymapInvPos13", mymapInvPos13);
    int mymapInvPos14[3] = { 0xf4, 0x83, 0x1e };
    this->addColor("_mymapInvPos14", mymapInvPos14);
    int mymapInvPos15[3] = { 0xf9, 0x8e, 0x23 };
    this->addColor("_mymapInvPos15", mymapInvPos15);
    int mymapInvPos16[3] = { 0xfd, 0x98, 0x28 };
    this->addColor("_mymapInvPos16", mymapInvPos16);
    int mymapInvPos17[3] = { 0xfe, 0xa3, 0x31 };
    this->addColor("_mymapInvPos17", mymapInvPos17);
    int mymapInvPos18[3] = { 0xfe, 0xad, 0x3b };
    this->addColor("_mymapInvPos18", mymapInvPos18);
    int mymapInvPos19[3] = { 0xfe, 0xb8, 0x45 };
    this->addColor("_mymapInvPos19", mymapInvPos19);
    int mymapInvPos20[3] = { 0xfe, 0xc3, 0x4e };
    this->addColor("_mymapInvPos20", mymapInvPos20);
    int mymapInvPos21[3] = { 0xfe, 0xcb, 0x5e };
    this->addColor("_mymapInvPos21", mymapInvPos21);
    int mymapInvPos22[3] = { 0xfe, 0xd3, 0x6f };
    this->addColor("_mymapInvPos22", mymapInvPos22);
    int mymapInvPos23[3] = { 0xfe, 0xda, 0x7f };
    this->addColor("_mymapInvPos23", mymapInvPos23);
    int mymapInvPos24[3] = { 0xfe, 0xe2, 0x90 };
    this->addColor("_mymapInvPos24", mymapInvPos24);
    int mymapInvPos25[3] = { 0xf1, 0xe5, 0x1c };
    this->addColor("_mymapInvPos25", mymapInvPos25);
    int mymapInvPos26[3] = { 0xee, 0xe5, 0x1b };
    this->addColor("_mymapInvPos26", mymapInvPos26);
    int mymapInvPos27[3] = { 0xec, 0xe4, 0x1a };
    this->addColor("_mymapInvPos27", mymapInvPos27);
    int mymapInvPos28[3] = { 0xe9, 0xe4, 0x19 };
    this->addColor("_mymapInvPos28", mymapInvPos28);
    int mymapInvPos29[3] = { 0xe7, 0xe4, 0x19 };
    this->addColor("_mymapInvPos29", mymapInvPos29);
    int mymapInvPos30[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapInvPos30", mymapInvPos30);
    int mymapInvPos31[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapInvPos31", mymapInvPos31);
    int mymapInvPos32[3] = { 0xe1, 0xe3, 0x18 };
    this->addColor("_mymapInvPos32", mymapInvPos32);
    int mymapInvPos33[3] = { 0xdf, 0xe3, 0x18 };
    this->addColor("_mymapInvPos33", mymapInvPos33);
    int mymapInvPos34[3] = { 0xdc, 0xe2, 0x18 };
    this->addColor("_mymapInvPos34", mymapInvPos34);
    int mymapInvPos35[3] = { 0xda, 0xe2, 0x18 };
    this->addColor("_mymapInvPos35", mymapInvPos35);
    int mymapInvPos36[3] = { 0xd7, 0xe2, 0x19 };
    this->addColor("_mymapInvPos36", mymapInvPos36);
    int mymapInvPos37[3] = { 0xd4, 0xe1, 0x1a };
    this->addColor("_mymapInvPos37", mymapInvPos37);
    int mymapInvPos38[3] = { 0xd2, 0xe1, 0x1b };
    this->addColor("_mymapInvPos38", mymapInvPos38);
    int mymapInvPos39[3] = { 0xcf, 0xe1, 0x1c };
    this->addColor("_mymapInvPos39", mymapInvPos39);
    int mymapInvPos40[3] = { 0xcd, 0xe0, 0x1d };
    this->addColor("_mymapInvPos40", mymapInvPos40);
    int mymapInvPos41[3] = { 0xca, 0xe0, 0x1e };
    this->addColor("_mymapInvPos41", mymapInvPos41);
    int mymapInvPos42[3] = { 0xc7, 0xe0, 0x1f };
    this->addColor("_mymapInvPos42", mymapInvPos42);
    int mymapInvPos43[3] = { 0xc5, 0xdf, 0x21 };
    this->addColor("_mymapInvPos43", mymapInvPos43);
    int mymapInvPos44[3] = { 0xc2, 0xdf, 0x22 };
    this->addColor("_mymapInvPos44", mymapInvPos44);
    int mymapInvPos45[3] = { 0xbf, 0xdf, 0x24 };
    this->addColor("_mymapInvPos45", mymapInvPos45);
    int mymapInvPos46[3] = { 0xbd, 0xde, 0x26 };
    this->addColor("_mymapInvPos46", mymapInvPos46);
    int mymapInvPos47[3] = { 0xba, 0xde, 0x27 };
    this->addColor("_mymapInvPos47", mymapInvPos47);
    int mymapInvPos48[3] = { 0xb7, 0xdd, 0x29 };
    this->addColor("_mymapInvPos48", mymapInvPos48);
    int mymapInvPos49[3] = { 0xb5, 0xdd, 0x2b };
    this->addColor("_mymapInvPos49", mymapInvPos49);
    int mymapInvPos50[3] = { 0xb2, 0xdd, 0x2c };
    this->addColor("_mymapInvPos50", mymapInvPos50);
    int mymapInvPos51[3] = { 0xaf, 0xdc, 0x2e };
    this->addColor("_mymapInvPos51", mymapInvPos51);
    int mymapInvPos52[3] = { 0xad, 0xdc, 0x30 };
    this->addColor("_mymapInvPos52", mymapInvPos52);
    int mymapInvPos53[3] = { 0xaa, 0xdb, 0x32 };
    this->addColor("_mymapInvPos53", mymapInvPos53);
    int mymapInvPos54[3] = { 0xa7, 0xdb, 0x33 };
    this->addColor("_mymapInvPos54", mymapInvPos54);
    int mymapInvPos55[3] = { 0xa5, 0xda, 0x35 };
    this->addColor("_mymapInvPos55", mymapInvPos55);
    int mymapInvPos56[3] = { 0xa2, 0xda, 0x37 };
    this->addColor("_mymapInvPos56", mymapInvPos56);
    int mymapInvPos57[3] = { 0x9f, 0xd9, 0x38 };
    this->addColor("_mymapInvPos57", mymapInvPos57);
    int mymapInvPos58[3] = { 0x9d, 0xd9, 0x3a };
    this->addColor("_mymapInvPos58", mymapInvPos58);
    int mymapInvPos59[3] = { 0x9a, 0xd8, 0x3c };
    this->addColor("_mymapInvPos59", mymapInvPos59);
    int mymapInvPos60[3] = { 0x97, 0xd8, 0x3e };
    this->addColor("_mymapInvPos60", mymapInvPos60);
    int mymapInvPos61[3] = { 0x95, 0xd7, 0x3f };
    this->addColor("_mymapInvPos61", mymapInvPos61);
    int mymapInvPos62[3] = { 0x92, 0xd7, 0x41 };
    this->addColor("_mymapInvPos62", mymapInvPos62);
    int mymapInvPos63[3] = { 0x90, 0xd6, 0x43 };
    this->addColor("_mymapInvPos63", mymapInvPos63);
    int mymapInvPos64[3] = { 0x8d, 0xd6, 0x44 };
    this->addColor("_mymapInvPos64", mymapInvPos64);
    int mymapInvPos65[3] = { 0x8b, 0xd5, 0x46 };
    this->addColor("_mymapInvPos65", mymapInvPos65);
    int mymapInvPos66[3] = { 0x88, 0xd5, 0x47 };
    this->addColor("_mymapInvPos66", mymapInvPos66);
    int mymapInvPos67[3] = { 0x86, 0xd4, 0x49 };
    this->addColor("_mymapInvPos67", mymapInvPos67);
    int mymapInvPos68[3] = { 0x83, 0xd3, 0x4b };
    this->addColor("_mymapInvPos68", mymapInvPos68);
    int mymapInvPos69[3] = { 0x81, 0xd3, 0x4c };
    this->addColor("_mymapInvPos69", mymapInvPos69);
    int mymapInvPos70[3] = { 0x7e, 0xd2, 0x4e };
    this->addColor("_mymapInvPos70", mymapInvPos70);
    int mymapInvPos71[3] = { 0x7c, 0xd2, 0x4f };
    this->addColor("_mymapInvPos71", mymapInvPos71);
    int mymapInvPos72[3] = { 0x79, 0xd1, 0x51 };
    this->addColor("_mymapInvPos72", mymapInvPos72);
    int mymapInvPos73[3] = { 0x77, 0xd0, 0x52 };
    this->addColor("_mymapInvPos73", mymapInvPos73);
    int mymapInvPos74[3] = { 0x74, 0xd0, 0x54 };
    this->addColor("_mymapInvPos74", mymapInvPos74);
    int mymapInvPos75[3] = { 0x72, 0xcf, 0x55 };
    this->addColor("_mymapInvPos75", mymapInvPos75);
    int mymapInvPos76[3] = { 0x70, 0xce, 0x56 };
    this->addColor("_mymapInvPos76", mymapInvPos76);
    int mymapInvPos77[3] = { 0x6d, 0xce, 0x58 };
    this->addColor("_mymapInvPos77", mymapInvPos77);
    int mymapInvPos78[3] = { 0x6b, 0xcd, 0x59 };
    this->addColor("_mymapInvPos78", mymapInvPos78);
    int mymapInvPos79[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapInvPos79", mymapInvPos79);
    int mymapInvPos80[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapInvPos80", mymapInvPos80);
    int mymapInvPos81[3] = { 0x67, 0xcc, 0x5c };
    this->addColor("_mymapInvPos81", mymapInvPos81);
    int mymapInvPos82[3] = { 0x64, 0xcb, 0x5d };
    this->addColor("_mymapInvPos82", mymapInvPos82);
    int mymapInvPos83[3] = { 0x62, 0xca, 0x5f };
    this->addColor("_mymapInvPos83", mymapInvPos83);
    int mymapInvPos84[3] = { 0x60, 0xc9, 0x60 };
    this->addColor("_mymapInvPos84", mymapInvPos84);
    int mymapInvPos85[3] = { 0x5e, 0xc9, 0x61 };
    this->addColor("_mymapInvPos85", mymapInvPos85);
    int mymapInvPos86[3] = { 0x5b, 0xc8, 0x62 };
    this->addColor("_mymapInvPos86", mymapInvPos86);
    int mymapInvPos87[3] = { 0x59, 0xc7, 0x64 };
    this->addColor("_mymapInvPos87", mymapInvPos87);
    int mymapInvPos88[3] = { 0x57, 0xc6, 0x65 };
    this->addColor("_mymapInvPos88", mymapInvPos88);
    int mymapInvPos89[3] = { 0x55, 0xc6, 0x66 };
    this->addColor("_mymapInvPos89", mymapInvPos89);
    int mymapInvPos90[3] = { 0x53, 0xc5, 0x67 };
    this->addColor("_mymapInvPos90", mymapInvPos90);
    int mymapInvPos91[3] = { 0x51, 0xc4, 0x68 };
    this->addColor("_mymapInvPos91", mymapInvPos91);
    int mymapInvPos92[3] = { 0x4f, 0xc3, 0x69 };
    this->addColor("_mymapInvPos92", mymapInvPos92);
    int mymapInvPos93[3] = { 0x4d, 0xc2, 0x6b };
    this->addColor("_mymapInvPos93", mymapInvPos93);
    int mymapInvPos94[3] = { 0x4b, 0xc2, 0x6c };
    this->addColor("_mymapInvPos94", mymapInvPos94);
    int mymapInvPos95[3] = { 0x49, 0xc1, 0x6d };
    this->addColor("_mymapInvPos95", mymapInvPos95);
    int mymapInvPos96[3] = { 0x47, 0xc0, 0x6e };
    this->addColor("_mymapInvPos96", mymapInvPos96);
    int mymapInvPos97[3] = { 0x45, 0xbf, 0x6f };
    this->addColor("_mymapInvPos97", mymapInvPos97);
    int mymapInvPos98[3] = { 0x44, 0xbe, 0x70 };
    this->addColor("_mymapInvPos98", mymapInvPos98);
    int mymapInvPos99[3] = { 0x42, 0xbe, 0x71 };
    this->addColor("_mymapInvPos99", mymapInvPos99);
    int mymapInvPos100[3] = { 0x40, 0xbd, 0x72 };
    this->addColor("_mymapInvPos100", mymapInvPos100);
    int mymapInvPos101[3] = { 0x3e, 0xbc, 0x73 };
    this->addColor("_mymapInvPos101", mymapInvPos101);
    int mymapInvPos102[3] = { 0x3d, 0xbb, 0x74 };
    this->addColor("_mymapInvPos102", mymapInvPos102);
    int mymapInvPos103[3] = { 0x3b, 0xba, 0x75 };
    this->addColor("_mymapInvPos103", mymapInvPos103);
    int mymapInvPos104[3] = { 0x39, 0xb9, 0x76 };
    this->addColor("_mymapInvPos104", mymapInvPos104);
    int mymapInvPos105[3] = { 0x38, 0xb9, 0x76 };
    this->addColor("_mymapInvPos105", mymapInvPos105);
    int mymapInvPos106[3] = { 0x36, 0xb8, 0x77 };
    this->addColor("_mymapInvPos106", mymapInvPos106);
    int mymapInvPos107[3] = { 0x35, 0xb7, 0x78 };
    this->addColor("_mymapInvPos107", mymapInvPos107);
    int mymapInvPos108[3] = { 0x33, 0xb6, 0x79 };
    this->addColor("_mymapInvPos108", mymapInvPos108);
    int mymapInvPos109[3] = { 0x32, 0xb5, 0x7a };
    this->addColor("_mymapInvPos109", mymapInvPos109);
    int mymapInvPos110[3] = { 0x30, 0xb4, 0x7a };
    this->addColor("_mymapInvPos110", mymapInvPos110);
    int mymapInvPos111[3] = { 0x2f, 0xb3, 0x7b };
    this->addColor("_mymapInvPos111", mymapInvPos111);
    int mymapInvPos112[3] = { 0x2e, 0xb2, 0x7c };
    this->addColor("_mymapInvPos112", mymapInvPos112);
    int mymapInvPos113[3] = { 0x2c, 0xb1, 0x7d };
    this->addColor("_mymapInvPos113", mymapInvPos113);
    int mymapInvPos114[3] = { 0x2b, 0xb1, 0x7d };
    this->addColor("_mymapInvPos114", mymapInvPos114);
    int mymapInvPos115[3] = { 0x2a, 0xb0, 0x7e };
    this->addColor("_mymapInvPos115", mymapInvPos115);
    int mymapInvPos116[3] = { 0x29, 0xaf, 0x7f };
    this->addColor("_mymapInvPos116", mymapInvPos116);
    int mymapInvPos117[3] = { 0x28, 0xae, 0x7f };
    this->addColor("_mymapInvPos117", mymapInvPos117);
    int mymapInvPos118[3] = { 0x27, 0xad, 0x80 };
    this->addColor("_mymapInvPos118", mymapInvPos118);
    int mymapInvPos119[3] = { 0x26, 0xac, 0x81 };
    this->addColor("_mymapInvPos119", mymapInvPos119);
    int mymapInvPos120[3] = { 0x25, 0xab, 0x81 };
    this->addColor("_mymapInvPos120", mymapInvPos120);
    int mymapInvPos121[3] = { 0x24, 0xaa, 0x82 };
    this->addColor("_mymapInvPos121", mymapInvPos121);
    int mymapInvPos122[3] = { 0x23, 0xa9, 0x82 };
    this->addColor("_mymapInvPos122", mymapInvPos122);
    int mymapInvPos123[3] = { 0x23, 0xa8, 0x83 };
    this->addColor("_mymapInvPos123", mymapInvPos123);
    int mymapInvPos124[3] = { 0x22, 0xa7, 0x84 };
    this->addColor("_mymapInvPos124", mymapInvPos124);
    int mymapInvPos125[3] = { 0x21, 0xa7, 0x84 };
    this->addColor("_mymapInvPos125", mymapInvPos125);
    int mymapInvPos126[3] = { 0x21, 0xa6, 0x85 };
    this->addColor("_mymapInvPos126", mymapInvPos126);
    int mymapInvPos127[3] = { 0x20, 0xa5, 0x85 };
    this->addColor("_mymapInvPos127", mymapInvPos127);
    int mymapInvPos128[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapInvPos128", mymapInvPos128);
    int mymapInvPos129[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapInvPos129", mymapInvPos129);
    int mymapInvPos130[3] = { 0x1f, 0xa3, 0x86 };
    this->addColor("_mymapInvPos130", mymapInvPos130);
    int mymapInvPos131[3] = { 0x1f, 0xa2, 0x86 };
    this->addColor("_mymapInvPos131", mymapInvPos131);
    int mymapInvPos132[3] = { 0x1f, 0xa1, 0x87 };
    this->addColor("_mymapInvPos132", mymapInvPos132);
    int mymapInvPos133[3] = { 0x1e, 0xa0, 0x87 };
    this->addColor("_mymapInvPos133", mymapInvPos133);
    int mymapInvPos134[3] = { 0x1e, 0x9f, 0x88 };
    this->addColor("_mymapInvPos134", mymapInvPos134);
    int mymapInvPos135[3] = { 0x1e, 0x9e, 0x88 };
    this->addColor("_mymapInvPos135", mymapInvPos135);
    int mymapInvPos136[3] = { 0x1e, 0x9d, 0x88 };
    this->addColor("_mymapInvPos136", mymapInvPos136);
    int mymapInvPos137[3] = { 0x1e, 0x9c, 0x89 };
    this->addColor("_mymapInvPos137", mymapInvPos137);
    int mymapInvPos138[3] = { 0x1e, 0x9b, 0x89 };
    this->addColor("_mymapInvPos138", mymapInvPos138);
    int mymapInvPos139[3] = { 0x1e, 0x9a, 0x89 };
    this->addColor("_mymapInvPos139", mymapInvPos139);
    int mymapInvPos140[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapInvPos140", mymapInvPos140);
    int mymapInvPos141[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapInvPos141", mymapInvPos141);
    int mymapInvPos142[3] = { 0x1e, 0x98, 0x8a };
    this->addColor("_mymapInvPos142", mymapInvPos142);
    int mymapInvPos143[3] = { 0x1e, 0x97, 0x8a };
    this->addColor("_mymapInvPos143", mymapInvPos143);
    int mymapInvPos144[3] = { 0x1f, 0x96, 0x8b };
    this->addColor("_mymapInvPos144", mymapInvPos144);
    int mymapInvPos145[3] = { 0x1f, 0x95, 0x8b };
    this->addColor("_mymapInvPos145", mymapInvPos145);
    int mymapInvPos146[3] = { 0x1f, 0x94, 0x8b };
    this->addColor("_mymapInvPos146", mymapInvPos146);
    int mymapInvPos147[3] = { 0x1f, 0x93, 0x8b };
    this->addColor("_mymapInvPos147", mymapInvPos147);
    int mymapInvPos148[3] = { 0x1f, 0x92, 0x8c };
    this->addColor("_mymapInvPos148", mymapInvPos148);
    int mymapInvPos149[3] = { 0x20, 0x91, 0x8c };
    this->addColor("_mymapInvPos149", mymapInvPos149);
    int mymapInvPos150[3] = { 0x20, 0x90, 0x8c };
    this->addColor("_mymapInvPos150", mymapInvPos150);
    int mymapInvPos151[3] = { 0x20, 0x8f, 0x8c };
    this->addColor("_mymapInvPos151", mymapInvPos151);
    int mymapInvPos152[3] = { 0x21, 0x8e, 0x8c };
    this->addColor("_mymapInvPos152", mymapInvPos152);
    int mymapInvPos153[3] = { 0x21, 0x8d, 0x8c };
    this->addColor("_mymapInvPos153", mymapInvPos153);
    int mymapInvPos154[3] = { 0x21, 0x8c, 0x8d };
    this->addColor("_mymapInvPos154", mymapInvPos154);
    int mymapInvPos155[3] = { 0x22, 0x8b, 0x8d };
    this->addColor("_mymapInvPos155", mymapInvPos155);
    int mymapInvPos156[3] = { 0x22, 0x8a, 0x8d };
    this->addColor("_mymapInvPos156", mymapInvPos156);
    int mymapInvPos157[3] = { 0x22, 0x89, 0x8d };
    this->addColor("_mymapInvPos157", mymapInvPos157);
    int mymapInvPos158[3] = { 0x23, 0x89, 0x8d };
    this->addColor("_mymapInvPos158", mymapInvPos158);
    int mymapInvPos159[3] = { 0x23, 0x88, 0x8d };
    this->addColor("_mymapInvPos159", mymapInvPos159);
    int mymapInvPos160[3] = { 0x23, 0x87, 0x8d };
    this->addColor("_mymapInvPos160", mymapInvPos160);
    int mymapInvPos161[3] = { 0x24, 0x86, 0x8d };
    this->addColor("_mymapInvPos161", mymapInvPos161);
    int mymapInvPos162[3] = { 0x24, 0x85, 0x8d };
    this->addColor("_mymapInvPos162", mymapInvPos162);
    int mymapInvPos163[3] = { 0x24, 0x84, 0x8d };
    this->addColor("_mymapInvPos163", mymapInvPos163);
    int mymapInvPos164[3] = { 0x25, 0x83, 0x8d };
    this->addColor("_mymapInvPos164", mymapInvPos164);
    int mymapInvPos165[3] = { 0x25, 0x82, 0x8e };
    this->addColor("_mymapInvPos165", mymapInvPos165);
    int mymapInvPos166[3] = { 0x26, 0x81, 0x8e };
    this->addColor("_mymapInvPos166", mymapInvPos166);
    int mymapInvPos167[3] = { 0x26, 0x80, 0x8e };
    this->addColor("_mymapInvPos167", mymapInvPos167);
    int mymapInvPos168[3] = { 0x26, 0x7f, 0x8e };
    this->addColor("_mymapInvPos168", mymapInvPos168);
    int mymapInvPos169[3] = { 0x27, 0x7e, 0x8e };
    this->addColor("_mymapInvPos169", mymapInvPos169);
    int mymapInvPos170[3] = { 0x27, 0x7d, 0x8e };
    this->addColor("_mymapInvPos170", mymapInvPos170);
    int mymapInvPos171[3] = { 0x27, 0x7c, 0x8e };
    this->addColor("_mymapInvPos171", mymapInvPos171);
    int mymapInvPos172[3] = { 0x28, 0x7b, 0x8e };
    this->addColor("_mymapInvPos172", mymapInvPos172);
    int mymapInvPos173[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapInvPos173", mymapInvPos173);
    int mymapInvPos174[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapInvPos174", mymapInvPos174);
    int mymapInvPos175[3] = { 0x29, 0x79, 0x8e };
    this->addColor("_mymapInvPos175", mymapInvPos175);
    int mymapInvPos176[3] = { 0x29, 0x78, 0x8e };
    this->addColor("_mymapInvPos176", mymapInvPos176);
    int mymapInvPos177[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapInvPos177", mymapInvPos177);
    int mymapInvPos178[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapInvPos178", mymapInvPos178);
    int mymapInvPos179[3] = { 0x2a, 0x76, 0x8e };
    this->addColor("_mymapInvPos179", mymapInvPos179);
    int mymapInvPos180[3] = { 0x2a, 0x75, 0x8e };
    this->addColor("_mymapInvPos180", mymapInvPos180);
    int mymapInvPos181[3] = { 0x2b, 0x74, 0x8e };
    this->addColor("_mymapInvPos181", mymapInvPos181);
    int mymapInvPos182[3] = { 0x2b, 0x73, 0x8e };
    this->addColor("_mymapInvPos182", mymapInvPos182);
    int mymapInvPos183[3] = { 0x2c, 0x72, 0x8e };
    this->addColor("_mymapInvPos183", mymapInvPos183);
    int mymapInvPos184[3] = { 0x2c, 0x71, 0x8e };
    this->addColor("_mymapInvPos184", mymapInvPos184);
    int mymapInvPos185[3] = { 0x2c, 0x70, 0x8e };
    this->addColor("_mymapInvPos185", mymapInvPos185);
    int mymapInvPos186[3] = { 0x2d, 0x6f, 0x8e };
    this->addColor("_mymapInvPos186", mymapInvPos186);
    int mymapInvPos187[3] = { 0x2d, 0x6e, 0x8e };
    this->addColor("_mymapInvPos187", mymapInvPos187);
    int mymapInvPos188[3] = { 0x2e, 0x6d, 0x8e };
    this->addColor("_mymapInvPos188", mymapInvPos188);
    int mymapInvPos189[3] = { 0x2e, 0x6c, 0x8e };
    this->addColor("_mymapInvPos189", mymapInvPos189);
    int mymapInvPos190[3] = { 0x2e, 0x6b, 0x8e };
    this->addColor("_mymapInvPos190", mymapInvPos190);
    int mymapInvPos191[3] = { 0x2f, 0x6a, 0x8d };
    this->addColor("_mymapInvPos191", mymapInvPos191);
    int mymapInvPos192[3] = { 0x2f, 0x69, 0x8d };
    this->addColor("_mymapInvPos192", mymapInvPos192);
    int mymapInvPos193[3] = { 0x30, 0x68, 0x8d };
    this->addColor("_mymapInvPos193", mymapInvPos193);
    int mymapInvPos194[3] = { 0x30, 0x67, 0x8d };
    this->addColor("_mymapInvPos194", mymapInvPos194);
    int mymapInvPos195[3] = { 0x31, 0x66, 0x8d };
    this->addColor("_mymapInvPos195", mymapInvPos195);
    int mymapInvPos196[3] = { 0x31, 0x65, 0x8d };
    this->addColor("_mymapInvPos196", mymapInvPos196);
    int mymapInvPos197[3] = { 0x31, 0x64, 0x8d };
    this->addColor("_mymapInvPos197", mymapInvPos197);
    int mymapInvPos198[3] = { 0x32, 0x63, 0x8d };
    this->addColor("_mymapInvPos198", mymapInvPos198);
    int mymapInvPos199[3] = { 0x32, 0x62, 0x8d };
    this->addColor("_mymapInvPos199", mymapInvPos199);
    int mymapInvPos200[3] = { 0x33, 0x61, 0x8d };
    this->addColor("_mymapInvPos200", mymapInvPos200);
    int mymapInvPos201[3] = { 0x33, 0x60, 0x8d };
    this->addColor("_mymapInvPos201", mymapInvPos201);
    int mymapInvPos202[3] = { 0x34, 0x5f, 0x8d };
    this->addColor("_mymapInvPos202", mymapInvPos202);
    int mymapInvPos203[3] = { 0x34, 0x5e, 0x8d };
    this->addColor("_mymapInvPos203", mymapInvPos203);
    int mymapInvPos204[3] = { 0x35, 0x5d, 0x8c };
    this->addColor("_mymapInvPos204", mymapInvPos204);
    int mymapInvPos205[3] = { 0x35, 0x5c, 0x8c };
    this->addColor("_mymapInvPos205", mymapInvPos205);
    int mymapInvPos206[3] = { 0x36, 0x5b, 0x8c };
    this->addColor("_mymapInvPos206", mymapInvPos206);
    int mymapInvPos207[3] = { 0x36, 0x5a, 0x8c };
    this->addColor("_mymapInvPos207", mymapInvPos207);
    int mymapInvPos208[3] = { 0x37, 0x59, 0x8c };
    this->addColor("_mymapInvPos208", mymapInvPos208);
    int mymapInvPos209[3] = { 0x37, 0x58, 0x8c };
    this->addColor("_mymapInvPos209", mymapInvPos209);
    int mymapInvPos210[3] = { 0x38, 0x57, 0x8c };
    this->addColor("_mymapInvPos210", mymapInvPos210);
    int mymapInvPos211[3] = { 0x38, 0x56, 0x8b };
    this->addColor("_mymapInvPos211", mymapInvPos211);
    int mymapInvPos212[3] = { 0x39, 0x55, 0x8b };
    this->addColor("_mymapInvPos212", mymapInvPos212);
    int mymapInvPos213[3] = { 0x39, 0x54, 0x8b };
    this->addColor("_mymapInvPos213", mymapInvPos213);
    int mymapInvPos214[3] = { 0x3a, 0x53, 0x8b };
    this->addColor("_mymapInvPos214", mymapInvPos214);
    int mymapInvPos215[3] = { 0x3a, 0x52, 0x8b };
    this->addColor("_mymapInvPos215", mymapInvPos215);
    int mymapInvPos216[3] = { 0x3b, 0x51, 0x8a };
    this->addColor("_mymapInvPos216", mymapInvPos216);
    int mymapInvPos217[3] = { 0x3b, 0x50, 0x8a };
    this->addColor("_mymapInvPos217", mymapInvPos217);
    int mymapInvPos218[3] = { 0x3c, 0x4e, 0x8a };
    this->addColor("_mymapInvPos218", mymapInvPos218);
    int mymapInvPos219[3] = { 0x3c, 0x4d, 0x8a };
    this->addColor("_mymapInvPos219", mymapInvPos219);
    int mymapInvPos220[3] = { 0x3d, 0x4c, 0x89 };
    this->addColor("_mymapInvPos220", mymapInvPos220);
    int mymapInvPos221[3] = { 0x3d, 0x4b, 0x89 };
    this->addColor("_mymapInvPos221", mymapInvPos221);
    int mymapInvPos222[3] = { 0x3d, 0x4a, 0x89 };
    this->addColor("_mymapInvPos222", mymapInvPos222);
    int mymapInvPos223[3] = { 0x3e, 0x49, 0x89 };
    this->addColor("_mymapInvPos223", mymapInvPos223);
    int mymapInvPos224[3] = { 0x3e, 0x48, 0x88 };
    this->addColor("_mymapInvPos224", mymapInvPos224);
    int mymapInvPos225[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapInvPos225", mymapInvPos225);
    int mymapInvPos226[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapInvPos226", mymapInvPos226);
    int mymapInvPos227[3] = { 0x3f, 0x45, 0x87 };
    this->addColor("_mymapInvPos227", mymapInvPos227);
    int mymapInvPos228[3] = { 0x40, 0x44, 0x87 };
    this->addColor("_mymapInvPos228", mymapInvPos228);
    int mymapInvPos229[3] = { 0x40, 0x43, 0x87 };
    this->addColor("_mymapInvPos229", mymapInvPos229);
    int mymapInvPos230[3] = { 0x41, 0x42, 0x86 };
    this->addColor("_mymapInvPos230", mymapInvPos230);
    int mymapInvPos231[3] = { 0x41, 0x41, 0x86 };
    this->addColor("_mymapInvPos231", mymapInvPos231);
    int mymapInvPos232[3] = { 0x42, 0x40, 0x85 };
    this->addColor("_mymapInvPos232", mymapInvPos232);
    int mymapInvPos233[3] = { 0x42, 0x3e, 0x85 };
    this->addColor("_mymapInvPos233", mymapInvPos233);
    int mymapInvPos234[3] = { 0x42, 0x3d, 0x84 };
    this->addColor("_mymapInvPos234", mymapInvPos234);
    int mymapInvPos235[3] = { 0x43, 0x3c, 0x84 };
    this->addColor("_mymapInvPos235", mymapInvPos235);
    int mymapInvPos236[3] = { 0x43, 0x3b, 0x83 };
    this->addColor("_mymapInvPos236", mymapInvPos236);
    int mymapInvPos237[3] = { 0x43, 0x3a, 0x83 };
    this->addColor("_mymapInvPos237", mymapInvPos237);
    int mymapInvPos238[3] = { 0x44, 0x39, 0x82 };
    this->addColor("_mymapInvPos238", mymapInvPos238);
    int mymapInvPos239[3] = { 0x44, 0x37, 0x81 };
    this->addColor("_mymapInvPos239", mymapInvPos239);
    int mymapInvPos240[3] = { 0x45, 0x36, 0x81 };
    this->addColor("_mymapInvPos240", mymapInvPos240);
    int mymapInvPos241[3] = { 0x45, 0x35, 0x80 };
    this->addColor("_mymapInvPos241", mymapInvPos241);
    int mymapInvPos242[3] = { 0x45, 0x34, 0x7f };
    this->addColor("_mymapInvPos242", mymapInvPos242);
    int mymapInvPos243[3] = { 0x45, 0x32, 0x7f };
    this->addColor("_mymapInvPos243", mymapInvPos243);
    int mymapInvPos244[3] = { 0x46, 0x31, 0x7e };
    this->addColor("_mymapInvPos244", mymapInvPos244);
    int mymapInvPos245[3] = { 0x46, 0x30, 0x7d };
    this->addColor("_mymapInvPos245", mymapInvPos245);
    int mymapInvPos246[3] = { 0x46, 0x2f, 0x7c };
    this->addColor("_mymapInvPos246", mymapInvPos246);
    int mymapInvPos247[3] = { 0x46, 0x2d, 0x7c };
    this->addColor("_mymapInvPos247", mymapInvPos247);
    int mymapInvPos248[3] = { 0x47, 0x2c, 0x7b };
    this->addColor("_mymapInvPos248", mymapInvPos248);
    int mymapInvPos249[3] = { 0x47, 0x2b, 0x7a };
    this->addColor("_mymapInvPos249", mymapInvPos249);
    int mymapInvPos250[3] = { 0x47, 0x2a, 0x79 };
    this->addColor("_mymapInvPos250", mymapInvPos250);
    int mymapInvPos251[3] = { 0x47, 0x28, 0x78 };
    this->addColor("_mymapInvPos251", mymapInvPos251);
    int mymapInvPos252[3] = { 0x47, 0x27, 0x77 };
    this->addColor("_mymapInvPos252", mymapInvPos252);
    int mymapInvPos253[3] = { 0x47, 0x26, 0x76 };
    this->addColor("_mymapInvPos253", mymapInvPos253);
    int mymapInvPos254[3] = { 0x47, 0x25, 0x75 };
    this->addColor("_mymapInvPos254", mymapInvPos254);
    int mymapInvPos255[3] = { 0x48, 0x23, 0x74 };
    this->addColor("_mymapInvPos255", mymapInvPos255);
    if (this->getPaletteByName("margulies_inv_pos") == NULL) {
        Palette mymapInvPos;
        mymapInvPos.setName("margulies_inv_pos");
        mymapInvPos.addScalarAndColor( 1.000000f, "_mymapInvPos0");
        mymapInvPos.addScalarAndColor( 0.996078f, "_mymapInvPos1");
        mymapInvPos.addScalarAndColor( 0.992157f, "_mymapInvPos2");
        mymapInvPos.addScalarAndColor( 0.988235f, "_mymapInvPos3");
        mymapInvPos.addScalarAndColor( 0.984314f, "_mymapInvPos4");
        mymapInvPos.addScalarAndColor( 0.980392f, "_mymapInvPos5");
        mymapInvPos.addScalarAndColor( 0.976471f, "_mymapInvPos6");
        mymapInvPos.addScalarAndColor( 0.972549f, "_mymapInvPos7");
        mymapInvPos.addScalarAndColor( 0.968627f, "_mymapInvPos8");
        mymapInvPos.addScalarAndColor( 0.964706f, "_mymapInvPos9");
        mymapInvPos.addScalarAndColor( 0.960784f, "_mymapInvPos10");
        mymapInvPos.addScalarAndColor( 0.956863f, "_mymapInvPos11");
        mymapInvPos.addScalarAndColor( 0.952941f, "_mymapInvPos12");
        mymapInvPos.addScalarAndColor( 0.949020f, "_mymapInvPos13");
        mymapInvPos.addScalarAndColor( 0.945098f, "_mymapInvPos14");
        mymapInvPos.addScalarAndColor( 0.941176f, "_mymapInvPos15");
        mymapInvPos.addScalarAndColor( 0.937255f, "_mymapInvPos16");
        mymapInvPos.addScalarAndColor( 0.933333f, "_mymapInvPos17");
        mymapInvPos.addScalarAndColor( 0.929412f, "_mymapInvPos18");
        mymapInvPos.addScalarAndColor( 0.925490f, "_mymapInvPos19");
        mymapInvPos.addScalarAndColor( 0.921569f, "_mymapInvPos20");
        mymapInvPos.addScalarAndColor( 0.917647f, "_mymapInvPos21");
        mymapInvPos.addScalarAndColor( 0.913725f, "_mymapInvPos22");
        mymapInvPos.addScalarAndColor( 0.909804f, "_mymapInvPos23");
        mymapInvPos.addScalarAndColor( 0.905882f, "_mymapInvPos24");
        mymapInvPos.addScalarAndColor( 0.901961f, "_mymapInvPos25");
        mymapInvPos.addScalarAndColor( 0.898039f, "_mymapInvPos26");
        mymapInvPos.addScalarAndColor( 0.894118f, "_mymapInvPos27");
        mymapInvPos.addScalarAndColor( 0.890196f, "_mymapInvPos28");
        mymapInvPos.addScalarAndColor( 0.886275f, "_mymapInvPos29");
        mymapInvPos.addScalarAndColor( 0.882353f, "_mymapInvPos30");
        mymapInvPos.addScalarAndColor( 0.878431f, "_mymapInvPos31");
        mymapInvPos.addScalarAndColor( 0.874510f, "_mymapInvPos32");
        mymapInvPos.addScalarAndColor( 0.870588f, "_mymapInvPos33");
        mymapInvPos.addScalarAndColor( 0.866667f, "_mymapInvPos34");
        mymapInvPos.addScalarAndColor( 0.862745f, "_mymapInvPos35");
        mymapInvPos.addScalarAndColor( 0.858824f, "_mymapInvPos36");
        mymapInvPos.addScalarAndColor( 0.854902f, "_mymapInvPos37");
        mymapInvPos.addScalarAndColor( 0.850980f, "_mymapInvPos38");
        mymapInvPos.addScalarAndColor( 0.847059f, "_mymapInvPos39");
        mymapInvPos.addScalarAndColor( 0.843137f, "_mymapInvPos40");
        mymapInvPos.addScalarAndColor( 0.839216f, "_mymapInvPos41");
        mymapInvPos.addScalarAndColor( 0.835294f, "_mymapInvPos42");
        mymapInvPos.addScalarAndColor( 0.831373f, "_mymapInvPos43");
        mymapInvPos.addScalarAndColor( 0.827451f, "_mymapInvPos44");
        mymapInvPos.addScalarAndColor( 0.823529f, "_mymapInvPos45");
        mymapInvPos.addScalarAndColor( 0.819608f, "_mymapInvPos46");
        mymapInvPos.addScalarAndColor( 0.815686f, "_mymapInvPos47");
        mymapInvPos.addScalarAndColor( 0.811765f, "_mymapInvPos48");
        mymapInvPos.addScalarAndColor( 0.807843f, "_mymapInvPos49");
        mymapInvPos.addScalarAndColor( 0.803922f, "_mymapInvPos50");
        mymapInvPos.addScalarAndColor( 0.800000f, "_mymapInvPos51");
        mymapInvPos.addScalarAndColor( 0.796078f, "_mymapInvPos52");
        mymapInvPos.addScalarAndColor( 0.792157f, "_mymapInvPos53");
        mymapInvPos.addScalarAndColor( 0.788235f, "_mymapInvPos54");
        mymapInvPos.addScalarAndColor( 0.784314f, "_mymapInvPos55");
        mymapInvPos.addScalarAndColor( 0.780392f, "_mymapInvPos56");
        mymapInvPos.addScalarAndColor( 0.776471f, "_mymapInvPos57");
        mymapInvPos.addScalarAndColor( 0.772549f, "_mymapInvPos58");
        mymapInvPos.addScalarAndColor( 0.768627f, "_mymapInvPos59");
        mymapInvPos.addScalarAndColor( 0.764706f, "_mymapInvPos60");
        mymapInvPos.addScalarAndColor( 0.760784f, "_mymapInvPos61");
        mymapInvPos.addScalarAndColor( 0.756863f, "_mymapInvPos62");
        mymapInvPos.addScalarAndColor( 0.752941f, "_mymapInvPos63");
        mymapInvPos.addScalarAndColor( 0.749020f, "_mymapInvPos64");
        mymapInvPos.addScalarAndColor( 0.745098f, "_mymapInvPos65");
        mymapInvPos.addScalarAndColor( 0.741176f, "_mymapInvPos66");
        mymapInvPos.addScalarAndColor( 0.737255f, "_mymapInvPos67");
        mymapInvPos.addScalarAndColor( 0.733333f, "_mymapInvPos68");
        mymapInvPos.addScalarAndColor( 0.729412f, "_mymapInvPos69");
        mymapInvPos.addScalarAndColor( 0.725490f, "_mymapInvPos70");
        mymapInvPos.addScalarAndColor( 0.721569f, "_mymapInvPos71");
        mymapInvPos.addScalarAndColor( 0.717647f, "_mymapInvPos72");
        mymapInvPos.addScalarAndColor( 0.713725f, "_mymapInvPos73");
        mymapInvPos.addScalarAndColor( 0.709804f, "_mymapInvPos74");
        mymapInvPos.addScalarAndColor( 0.705882f, "_mymapInvPos75");
        mymapInvPos.addScalarAndColor( 0.701961f, "_mymapInvPos76");
        mymapInvPos.addScalarAndColor( 0.698039f, "_mymapInvPos77");
        mymapInvPos.addScalarAndColor( 0.694118f, "_mymapInvPos78");
        mymapInvPos.addScalarAndColor( 0.690196f, "_mymapInvPos79");
        mymapInvPos.addScalarAndColor( 0.686275f, "_mymapInvPos80");
        mymapInvPos.addScalarAndColor( 0.682353f, "_mymapInvPos81");
        mymapInvPos.addScalarAndColor( 0.678431f, "_mymapInvPos82");
        mymapInvPos.addScalarAndColor( 0.674510f, "_mymapInvPos83");
        mymapInvPos.addScalarAndColor( 0.670588f, "_mymapInvPos84");
        mymapInvPos.addScalarAndColor( 0.666667f, "_mymapInvPos85");
        mymapInvPos.addScalarAndColor( 0.662745f, "_mymapInvPos86");
        mymapInvPos.addScalarAndColor( 0.658824f, "_mymapInvPos87");
        mymapInvPos.addScalarAndColor( 0.654902f, "_mymapInvPos88");
        mymapInvPos.addScalarAndColor( 0.650980f, "_mymapInvPos89");
        mymapInvPos.addScalarAndColor( 0.647059f, "_mymapInvPos90");
        mymapInvPos.addScalarAndColor( 0.643137f, "_mymapInvPos91");
        mymapInvPos.addScalarAndColor( 0.639216f, "_mymapInvPos92");
        mymapInvPos.addScalarAndColor( 0.635294f, "_mymapInvPos93");
        mymapInvPos.addScalarAndColor( 0.631373f, "_mymapInvPos94");
        mymapInvPos.addScalarAndColor( 0.627451f, "_mymapInvPos95");
        mymapInvPos.addScalarAndColor( 0.623529f, "_mymapInvPos96");
        mymapInvPos.addScalarAndColor( 0.619608f, "_mymapInvPos97");
        mymapInvPos.addScalarAndColor( 0.615686f, "_mymapInvPos98");
        mymapInvPos.addScalarAndColor( 0.611765f, "_mymapInvPos99");
        mymapInvPos.addScalarAndColor( 0.607843f, "_mymapInvPos100");
        mymapInvPos.addScalarAndColor( 0.603922f, "_mymapInvPos101");
        mymapInvPos.addScalarAndColor( 0.600000f, "_mymapInvPos102");
        mymapInvPos.addScalarAndColor( 0.596078f, "_mymapInvPos103");
        mymapInvPos.addScalarAndColor( 0.592157f, "_mymapInvPos104");
        mymapInvPos.addScalarAndColor( 0.588235f, "_mymapInvPos105");
        mymapInvPos.addScalarAndColor( 0.584314f, "_mymapInvPos106");
        mymapInvPos.addScalarAndColor( 0.580392f, "_mymapInvPos107");
        mymapInvPos.addScalarAndColor( 0.576471f, "_mymapInvPos108");
        mymapInvPos.addScalarAndColor( 0.572549f, "_mymapInvPos109");
        mymapInvPos.addScalarAndColor( 0.568627f, "_mymapInvPos110");
        mymapInvPos.addScalarAndColor( 0.564706f, "_mymapInvPos111");
        mymapInvPos.addScalarAndColor( 0.560784f, "_mymapInvPos112");
        mymapInvPos.addScalarAndColor( 0.556863f, "_mymapInvPos113");
        mymapInvPos.addScalarAndColor( 0.552941f, "_mymapInvPos114");
        mymapInvPos.addScalarAndColor( 0.549020f, "_mymapInvPos115");
        mymapInvPos.addScalarAndColor( 0.545098f, "_mymapInvPos116");
        mymapInvPos.addScalarAndColor( 0.541176f, "_mymapInvPos117");
        mymapInvPos.addScalarAndColor( 0.537255f, "_mymapInvPos118");
        mymapInvPos.addScalarAndColor( 0.533333f, "_mymapInvPos119");
        mymapInvPos.addScalarAndColor( 0.529412f, "_mymapInvPos120");
        mymapInvPos.addScalarAndColor( 0.525490f, "_mymapInvPos121");
        mymapInvPos.addScalarAndColor( 0.521569f, "_mymapInvPos122");
        mymapInvPos.addScalarAndColor( 0.517647f, "_mymapInvPos123");
        mymapInvPos.addScalarAndColor( 0.513725f, "_mymapInvPos124");
        mymapInvPos.addScalarAndColor( 0.509804f, "_mymapInvPos125");
        mymapInvPos.addScalarAndColor( 0.505882f, "_mymapInvPos126");
        mymapInvPos.addScalarAndColor( 0.501961f, "_mymapInvPos127");
        mymapInvPos.addScalarAndColor( 0.498039f, "_mymapInvPos128");
        mymapInvPos.addScalarAndColor( 0.494118f, "_mymapInvPos129");
        mymapInvPos.addScalarAndColor( 0.490196f, "_mymapInvPos130");
        mymapInvPos.addScalarAndColor( 0.486275f, "_mymapInvPos131");
        mymapInvPos.addScalarAndColor( 0.482353f, "_mymapInvPos132");
        mymapInvPos.addScalarAndColor( 0.478431f, "_mymapInvPos133");
        mymapInvPos.addScalarAndColor( 0.474510f, "_mymapInvPos134");
        mymapInvPos.addScalarAndColor( 0.470588f, "_mymapInvPos135");
        mymapInvPos.addScalarAndColor( 0.466667f, "_mymapInvPos136");
        mymapInvPos.addScalarAndColor( 0.462745f, "_mymapInvPos137");
        mymapInvPos.addScalarAndColor( 0.458824f, "_mymapInvPos138");
        mymapInvPos.addScalarAndColor( 0.454902f, "_mymapInvPos139");
        mymapInvPos.addScalarAndColor( 0.450980f, "_mymapInvPos140");
        mymapInvPos.addScalarAndColor( 0.447059f, "_mymapInvPos141");
        mymapInvPos.addScalarAndColor( 0.443137f, "_mymapInvPos142");
        mymapInvPos.addScalarAndColor( 0.439216f, "_mymapInvPos143");
        mymapInvPos.addScalarAndColor( 0.435294f, "_mymapInvPos144");
        mymapInvPos.addScalarAndColor( 0.431373f, "_mymapInvPos145");
        mymapInvPos.addScalarAndColor( 0.427451f, "_mymapInvPos146");
        mymapInvPos.addScalarAndColor( 0.423529f, "_mymapInvPos147");
        mymapInvPos.addScalarAndColor( 0.419608f, "_mymapInvPos148");
        mymapInvPos.addScalarAndColor( 0.415686f, "_mymapInvPos149");
        mymapInvPos.addScalarAndColor( 0.411765f, "_mymapInvPos150");
        mymapInvPos.addScalarAndColor( 0.407843f, "_mymapInvPos151");
        mymapInvPos.addScalarAndColor( 0.403922f, "_mymapInvPos152");
        mymapInvPos.addScalarAndColor( 0.400000f, "_mymapInvPos153");
        mymapInvPos.addScalarAndColor( 0.396078f, "_mymapInvPos154");
        mymapInvPos.addScalarAndColor( 0.392157f, "_mymapInvPos155");
        mymapInvPos.addScalarAndColor( 0.388235f, "_mymapInvPos156");
        mymapInvPos.addScalarAndColor( 0.384314f, "_mymapInvPos157");
        mymapInvPos.addScalarAndColor( 0.380392f, "_mymapInvPos158");
        mymapInvPos.addScalarAndColor( 0.376471f, "_mymapInvPos159");
        mymapInvPos.addScalarAndColor( 0.372549f, "_mymapInvPos160");
        mymapInvPos.addScalarAndColor( 0.368627f, "_mymapInvPos161");
        mymapInvPos.addScalarAndColor( 0.364706f, "_mymapInvPos162");
        mymapInvPos.addScalarAndColor( 0.360784f, "_mymapInvPos163");
        mymapInvPos.addScalarAndColor( 0.356863f, "_mymapInvPos164");
        mymapInvPos.addScalarAndColor( 0.352941f, "_mymapInvPos165");
        mymapInvPos.addScalarAndColor( 0.349020f, "_mymapInvPos166");
        mymapInvPos.addScalarAndColor( 0.345098f, "_mymapInvPos167");
        mymapInvPos.addScalarAndColor( 0.341176f, "_mymapInvPos168");
        mymapInvPos.addScalarAndColor( 0.337255f, "_mymapInvPos169");
        mymapInvPos.addScalarAndColor( 0.333333f, "_mymapInvPos170");
        mymapInvPos.addScalarAndColor( 0.329412f, "_mymapInvPos171");
        mymapInvPos.addScalarAndColor( 0.325490f, "_mymapInvPos172");
        mymapInvPos.addScalarAndColor( 0.321569f, "_mymapInvPos173");
        mymapInvPos.addScalarAndColor( 0.317647f, "_mymapInvPos174");
        mymapInvPos.addScalarAndColor( 0.313725f, "_mymapInvPos175");
        mymapInvPos.addScalarAndColor( 0.309804f, "_mymapInvPos176");
        mymapInvPos.addScalarAndColor( 0.305882f, "_mymapInvPos177");
        mymapInvPos.addScalarAndColor( 0.301961f, "_mymapInvPos178");
        mymapInvPos.addScalarAndColor( 0.298039f, "_mymapInvPos179");
        mymapInvPos.addScalarAndColor( 0.294118f, "_mymapInvPos180");
        mymapInvPos.addScalarAndColor( 0.290196f, "_mymapInvPos181");
        mymapInvPos.addScalarAndColor( 0.286275f, "_mymapInvPos182");
        mymapInvPos.addScalarAndColor( 0.282353f, "_mymapInvPos183");
        mymapInvPos.addScalarAndColor( 0.278431f, "_mymapInvPos184");
        mymapInvPos.addScalarAndColor( 0.274510f, "_mymapInvPos185");
        mymapInvPos.addScalarAndColor( 0.270588f, "_mymapInvPos186");
        mymapInvPos.addScalarAndColor( 0.266667f, "_mymapInvPos187");
        mymapInvPos.addScalarAndColor( 0.262745f, "_mymapInvPos188");
        mymapInvPos.addScalarAndColor( 0.258824f, "_mymapInvPos189");
        mymapInvPos.addScalarAndColor( 0.254902f, "_mymapInvPos190");
        mymapInvPos.addScalarAndColor( 0.250980f, "_mymapInvPos191");
        mymapInvPos.addScalarAndColor( 0.247059f, "_mymapInvPos192");
        mymapInvPos.addScalarAndColor( 0.243137f, "_mymapInvPos193");
        mymapInvPos.addScalarAndColor( 0.239216f, "_mymapInvPos194");
        mymapInvPos.addScalarAndColor( 0.235294f, "_mymapInvPos195");
        mymapInvPos.addScalarAndColor( 0.231373f, "_mymapInvPos196");
        mymapInvPos.addScalarAndColor( 0.227451f, "_mymapInvPos197");
        mymapInvPos.addScalarAndColor( 0.223529f, "_mymapInvPos198");
        mymapInvPos.addScalarAndColor( 0.219608f, "_mymapInvPos199");
        mymapInvPos.addScalarAndColor( 0.215686f, "_mymapInvPos200");
        mymapInvPos.addScalarAndColor( 0.211765f, "_mymapInvPos201");
        mymapInvPos.addScalarAndColor( 0.207843f, "_mymapInvPos202");
        mymapInvPos.addScalarAndColor( 0.203922f, "_mymapInvPos203");
        mymapInvPos.addScalarAndColor( 0.200000f, "_mymapInvPos204");
        mymapInvPos.addScalarAndColor( 0.196078f, "_mymapInvPos205");
        mymapInvPos.addScalarAndColor( 0.192157f, "_mymapInvPos206");
        mymapInvPos.addScalarAndColor( 0.188235f, "_mymapInvPos207");
        mymapInvPos.addScalarAndColor( 0.184314f, "_mymapInvPos208");
        mymapInvPos.addScalarAndColor( 0.180392f, "_mymapInvPos209");
        mymapInvPos.addScalarAndColor( 0.176471f, "_mymapInvPos210");
        mymapInvPos.addScalarAndColor( 0.172549f, "_mymapInvPos211");
        mymapInvPos.addScalarAndColor( 0.168627f, "_mymapInvPos212");
        mymapInvPos.addScalarAndColor( 0.164706f, "_mymapInvPos213");
        mymapInvPos.addScalarAndColor( 0.160784f, "_mymapInvPos214");
        mymapInvPos.addScalarAndColor( 0.156863f, "_mymapInvPos215");
        mymapInvPos.addScalarAndColor( 0.152941f, "_mymapInvPos216");
        mymapInvPos.addScalarAndColor( 0.149020f, "_mymapInvPos217");
        mymapInvPos.addScalarAndColor( 0.145098f, "_mymapInvPos218");
        mymapInvPos.addScalarAndColor( 0.141176f, "_mymapInvPos219");
        mymapInvPos.addScalarAndColor( 0.137255f, "_mymapInvPos220");
        mymapInvPos.addScalarAndColor( 0.133333f, "_mymapInvPos221");
        mymapInvPos.addScalarAndColor( 0.129412f, "_mymapInvPos222");
        mymapInvPos.addScalarAndColor( 0.125490f, "_mymapInvPos223");
        mymapInvPos.addScalarAndColor( 0.121569f, "_mymapInvPos224");
        mymapInvPos.addScalarAndColor( 0.117647f, "_mymapInvPos225");
        mymapInvPos.addScalarAndColor( 0.113725f, "_mymapInvPos226");
        mymapInvPos.addScalarAndColor( 0.109804f, "_mymapInvPos227");
        mymapInvPos.addScalarAndColor( 0.105882f, "_mymapInvPos228");
        mymapInvPos.addScalarAndColor( 0.101961f, "_mymapInvPos229");
        mymapInvPos.addScalarAndColor( 0.098039f, "_mymapInvPos230");
        mymapInvPos.addScalarAndColor( 0.094118f, "_mymapInvPos231");
        mymapInvPos.addScalarAndColor( 0.090196f, "_mymapInvPos232");
        mymapInvPos.addScalarAndColor( 0.086275f, "_mymapInvPos233");
        mymapInvPos.addScalarAndColor( 0.082353f, "_mymapInvPos234");
        mymapInvPos.addScalarAndColor( 0.078431f, "_mymapInvPos235");
        mymapInvPos.addScalarAndColor( 0.074510f, "_mymapInvPos236");
        mymapInvPos.addScalarAndColor( 0.070588f, "_mymapInvPos237");
        mymapInvPos.addScalarAndColor( 0.066667f, "_mymapInvPos238");
        mymapInvPos.addScalarAndColor( 0.062745f, "_mymapInvPos239");
        mymapInvPos.addScalarAndColor( 0.058824f, "_mymapInvPos240");
        mymapInvPos.addScalarAndColor( 0.054902f, "_mymapInvPos241");
        mymapInvPos.addScalarAndColor( 0.050980f, "_mymapInvPos242");
        mymapInvPos.addScalarAndColor( 0.047059f, "_mymapInvPos243");
        mymapInvPos.addScalarAndColor( 0.043137f, "_mymapInvPos244");
        mymapInvPos.addScalarAndColor( 0.039216f, "_mymapInvPos245");
        mymapInvPos.addScalarAndColor( 0.035294f, "_mymapInvPos246");
        mymapInvPos.addScalarAndColor( 0.031373f, "_mymapInvPos247");
        mymapInvPos.addScalarAndColor( 0.027451f, "_mymapInvPos248");
        mymapInvPos.addScalarAndColor( 0.023529f, "_mymapInvPos249");
        mymapInvPos.addScalarAndColor( 0.019608f, "_mymapInvPos250");
        mymapInvPos.addScalarAndColor( 0.015686f, "_mymapInvPos251");
        mymapInvPos.addScalarAndColor( 0.011765f, "_mymapInvPos252");
        mymapInvPos.addScalarAndColor( 0.007843f, "_mymapInvPos253");
        mymapInvPos.addScalarAndColor( 0.003922f, "_mymapInvPos254");
        mymapInvPos.addScalarAndColor( 0.000000f, "_mymapInvPos255");
        addPalette(mymapInvPos);
    }
    int mymapPos0[3] = { 0x48, 0x23, 0x74 };
    this->addColor("_mymapPos0", mymapPos0);
    int mymapPos1[3] = { 0x47, 0x25, 0x75 };
    this->addColor("_mymapPos1", mymapPos1);
    int mymapPos2[3] = { 0x47, 0x26, 0x76 };
    this->addColor("_mymapPos2", mymapPos2);
    int mymapPos3[3] = { 0x47, 0x27, 0x77 };
    this->addColor("_mymapPos3", mymapPos3);
    int mymapPos4[3] = { 0x47, 0x28, 0x78 };
    this->addColor("_mymapPos4", mymapPos4);
    int mymapPos5[3] = { 0x47, 0x2a, 0x79 };
    this->addColor("_mymapPos5", mymapPos5);
    int mymapPos6[3] = { 0x47, 0x2b, 0x7a };
    this->addColor("_mymapPos6", mymapPos6);
    int mymapPos7[3] = { 0x47, 0x2c, 0x7b };
    this->addColor("_mymapPos7", mymapPos7);
    int mymapPos8[3] = { 0x46, 0x2d, 0x7c };
    this->addColor("_mymapPos8", mymapPos8);
    int mymapPos9[3] = { 0x46, 0x2f, 0x7c };
    this->addColor("_mymapPos9", mymapPos9);
    int mymapPos10[3] = { 0x46, 0x30, 0x7d };
    this->addColor("_mymapPos10", mymapPos10);
    int mymapPos11[3] = { 0x46, 0x31, 0x7e };
    this->addColor("_mymapPos11", mymapPos11);
    int mymapPos12[3] = { 0x45, 0x32, 0x7f };
    this->addColor("_mymapPos12", mymapPos12);
    int mymapPos13[3] = { 0x45, 0x34, 0x7f };
    this->addColor("_mymapPos13", mymapPos13);
    int mymapPos14[3] = { 0x45, 0x35, 0x80 };
    this->addColor("_mymapPos14", mymapPos14);
    int mymapPos15[3] = { 0x45, 0x36, 0x81 };
    this->addColor("_mymapPos15", mymapPos15);
    int mymapPos16[3] = { 0x44, 0x37, 0x81 };
    this->addColor("_mymapPos16", mymapPos16);
    int mymapPos17[3] = { 0x44, 0x39, 0x82 };
    this->addColor("_mymapPos17", mymapPos17);
    int mymapPos18[3] = { 0x43, 0x3a, 0x83 };
    this->addColor("_mymapPos18", mymapPos18);
    int mymapPos19[3] = { 0x43, 0x3b, 0x83 };
    this->addColor("_mymapPos19", mymapPos19);
    int mymapPos20[3] = { 0x43, 0x3c, 0x84 };
    this->addColor("_mymapPos20", mymapPos20);
    int mymapPos21[3] = { 0x42, 0x3d, 0x84 };
    this->addColor("_mymapPos21", mymapPos21);
    int mymapPos22[3] = { 0x42, 0x3e, 0x85 };
    this->addColor("_mymapPos22", mymapPos22);
    int mymapPos23[3] = { 0x42, 0x40, 0x85 };
    this->addColor("_mymapPos23", mymapPos23);
    int mymapPos24[3] = { 0x41, 0x41, 0x86 };
    this->addColor("_mymapPos24", mymapPos24);
    int mymapPos25[3] = { 0x41, 0x42, 0x86 };
    this->addColor("_mymapPos25", mymapPos25);
    int mymapPos26[3] = { 0x40, 0x43, 0x87 };
    this->addColor("_mymapPos26", mymapPos26);
    int mymapPos27[3] = { 0x40, 0x44, 0x87 };
    this->addColor("_mymapPos27", mymapPos27);
    int mymapPos28[3] = { 0x3f, 0x45, 0x87 };
    this->addColor("_mymapPos28", mymapPos28);
    int mymapPos29[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapPos29", mymapPos29);
    int mymapPos30[3] = { 0x3f, 0x47, 0x88 };
    this->addColor("_mymapPos30", mymapPos30);
    int mymapPos31[3] = { 0x3e, 0x48, 0x88 };
    this->addColor("_mymapPos31", mymapPos31);
    int mymapPos32[3] = { 0x3e, 0x49, 0x89 };
    this->addColor("_mymapPos32", mymapPos32);
    int mymapPos33[3] = { 0x3d, 0x4a, 0x89 };
    this->addColor("_mymapPos33", mymapPos33);
    int mymapPos34[3] = { 0x3d, 0x4b, 0x89 };
    this->addColor("_mymapPos34", mymapPos34);
    int mymapPos35[3] = { 0x3d, 0x4c, 0x89 };
    this->addColor("_mymapPos35", mymapPos35);
    int mymapPos36[3] = { 0x3c, 0x4d, 0x8a };
    this->addColor("_mymapPos36", mymapPos36);
    int mymapPos37[3] = { 0x3c, 0x4e, 0x8a };
    this->addColor("_mymapPos37", mymapPos37);
    int mymapPos38[3] = { 0x3b, 0x50, 0x8a };
    this->addColor("_mymapPos38", mymapPos38);
    int mymapPos39[3] = { 0x3b, 0x51, 0x8a };
    this->addColor("_mymapPos39", mymapPos39);
    int mymapPos40[3] = { 0x3a, 0x52, 0x8b };
    this->addColor("_mymapPos40", mymapPos40);
    int mymapPos41[3] = { 0x3a, 0x53, 0x8b };
    this->addColor("_mymapPos41", mymapPos41);
    int mymapPos42[3] = { 0x39, 0x54, 0x8b };
    this->addColor("_mymapPos42", mymapPos42);
    int mymapPos43[3] = { 0x39, 0x55, 0x8b };
    this->addColor("_mymapPos43", mymapPos43);
    int mymapPos44[3] = { 0x38, 0x56, 0x8b };
    this->addColor("_mymapPos44", mymapPos44);
    int mymapPos45[3] = { 0x38, 0x57, 0x8c };
    this->addColor("_mymapPos45", mymapPos45);
    int mymapPos46[3] = { 0x37, 0x58, 0x8c };
    this->addColor("_mymapPos46", mymapPos46);
    int mymapPos47[3] = { 0x37, 0x59, 0x8c };
    this->addColor("_mymapPos47", mymapPos47);
    int mymapPos48[3] = { 0x36, 0x5a, 0x8c };
    this->addColor("_mymapPos48", mymapPos48);
    int mymapPos49[3] = { 0x36, 0x5b, 0x8c };
    this->addColor("_mymapPos49", mymapPos49);
    int mymapPos50[3] = { 0x35, 0x5c, 0x8c };
    this->addColor("_mymapPos50", mymapPos50);
    int mymapPos51[3] = { 0x35, 0x5d, 0x8c };
    this->addColor("_mymapPos51", mymapPos51);
    int mymapPos52[3] = { 0x34, 0x5e, 0x8d };
    this->addColor("_mymapPos52", mymapPos52);
    int mymapPos53[3] = { 0x34, 0x5f, 0x8d };
    this->addColor("_mymapPos53", mymapPos53);
    int mymapPos54[3] = { 0x33, 0x60, 0x8d };
    this->addColor("_mymapPos54", mymapPos54);
    int mymapPos55[3] = { 0x33, 0x61, 0x8d };
    this->addColor("_mymapPos55", mymapPos55);
    int mymapPos56[3] = { 0x32, 0x62, 0x8d };
    this->addColor("_mymapPos56", mymapPos56);
    int mymapPos57[3] = { 0x32, 0x63, 0x8d };
    this->addColor("_mymapPos57", mymapPos57);
    int mymapPos58[3] = { 0x31, 0x64, 0x8d };
    this->addColor("_mymapPos58", mymapPos58);
    int mymapPos59[3] = { 0x31, 0x65, 0x8d };
    this->addColor("_mymapPos59", mymapPos59);
    int mymapPos60[3] = { 0x31, 0x66, 0x8d };
    this->addColor("_mymapPos60", mymapPos60);
    int mymapPos61[3] = { 0x30, 0x67, 0x8d };
    this->addColor("_mymapPos61", mymapPos61);
    int mymapPos62[3] = { 0x30, 0x68, 0x8d };
    this->addColor("_mymapPos62", mymapPos62);
    int mymapPos63[3] = { 0x2f, 0x69, 0x8d };
    this->addColor("_mymapPos63", mymapPos63);
    int mymapPos64[3] = { 0x2f, 0x6a, 0x8d };
    this->addColor("_mymapPos64", mymapPos64);
    int mymapPos65[3] = { 0x2e, 0x6b, 0x8e };
    this->addColor("_mymapPos65", mymapPos65);
    int mymapPos66[3] = { 0x2e, 0x6c, 0x8e };
    this->addColor("_mymapPos66", mymapPos66);
    int mymapPos67[3] = { 0x2e, 0x6d, 0x8e };
    this->addColor("_mymapPos67", mymapPos67);
    int mymapPos68[3] = { 0x2d, 0x6e, 0x8e };
    this->addColor("_mymapPos68", mymapPos68);
    int mymapPos69[3] = { 0x2d, 0x6f, 0x8e };
    this->addColor("_mymapPos69", mymapPos69);
    int mymapPos70[3] = { 0x2c, 0x70, 0x8e };
    this->addColor("_mymapPos70", mymapPos70);
    int mymapPos71[3] = { 0x2c, 0x71, 0x8e };
    this->addColor("_mymapPos71", mymapPos71);
    int mymapPos72[3] = { 0x2c, 0x72, 0x8e };
    this->addColor("_mymapPos72", mymapPos72);
    int mymapPos73[3] = { 0x2b, 0x73, 0x8e };
    this->addColor("_mymapPos73", mymapPos73);
    int mymapPos74[3] = { 0x2b, 0x74, 0x8e };
    this->addColor("_mymapPos74", mymapPos74);
    int mymapPos75[3] = { 0x2a, 0x75, 0x8e };
    this->addColor("_mymapPos75", mymapPos75);
    int mymapPos76[3] = { 0x2a, 0x76, 0x8e };
    this->addColor("_mymapPos76", mymapPos76);
    int mymapPos77[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapPos77", mymapPos77);
    int mymapPos78[3] = { 0x2a, 0x77, 0x8e };
    this->addColor("_mymapPos78", mymapPos78);
    int mymapPos79[3] = { 0x29, 0x78, 0x8e };
    this->addColor("_mymapPos79", mymapPos79);
    int mymapPos80[3] = { 0x29, 0x79, 0x8e };
    this->addColor("_mymapPos80", mymapPos80);
    int mymapPos81[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapPos81", mymapPos81);
    int mymapPos82[3] = { 0x28, 0x7a, 0x8e };
    this->addColor("_mymapPos82", mymapPos82);
    int mymapPos83[3] = { 0x28, 0x7b, 0x8e };
    this->addColor("_mymapPos83", mymapPos83);
    int mymapPos84[3] = { 0x27, 0x7c, 0x8e };
    this->addColor("_mymapPos84", mymapPos84);
    int mymapPos85[3] = { 0x27, 0x7d, 0x8e };
    this->addColor("_mymapPos85", mymapPos85);
    int mymapPos86[3] = { 0x27, 0x7e, 0x8e };
    this->addColor("_mymapPos86", mymapPos86);
    int mymapPos87[3] = { 0x26, 0x7f, 0x8e };
    this->addColor("_mymapPos87", mymapPos87);
    int mymapPos88[3] = { 0x26, 0x80, 0x8e };
    this->addColor("_mymapPos88", mymapPos88);
    int mymapPos89[3] = { 0x26, 0x81, 0x8e };
    this->addColor("_mymapPos89", mymapPos89);
    int mymapPos90[3] = { 0x25, 0x82, 0x8e };
    this->addColor("_mymapPos90", mymapPos90);
    int mymapPos91[3] = { 0x25, 0x83, 0x8d };
    this->addColor("_mymapPos91", mymapPos91);
    int mymapPos92[3] = { 0x24, 0x84, 0x8d };
    this->addColor("_mymapPos92", mymapPos92);
    int mymapPos93[3] = { 0x24, 0x85, 0x8d };
    this->addColor("_mymapPos93", mymapPos93);
    int mymapPos94[3] = { 0x24, 0x86, 0x8d };
    this->addColor("_mymapPos94", mymapPos94);
    int mymapPos95[3] = { 0x23, 0x87, 0x8d };
    this->addColor("_mymapPos95", mymapPos95);
    int mymapPos96[3] = { 0x23, 0x88, 0x8d };
    this->addColor("_mymapPos96", mymapPos96);
    int mymapPos97[3] = { 0x23, 0x89, 0x8d };
    this->addColor("_mymapPos97", mymapPos97);
    int mymapPos98[3] = { 0x22, 0x89, 0x8d };
    this->addColor("_mymapPos98", mymapPos98);
    int mymapPos99[3] = { 0x22, 0x8a, 0x8d };
    this->addColor("_mymapPos99", mymapPos99);
    int mymapPos100[3] = { 0x22, 0x8b, 0x8d };
    this->addColor("_mymapPos100", mymapPos100);
    int mymapPos101[3] = { 0x21, 0x8c, 0x8d };
    this->addColor("_mymapPos101", mymapPos101);
    int mymapPos102[3] = { 0x21, 0x8d, 0x8c };
    this->addColor("_mymapPos102", mymapPos102);
    int mymapPos103[3] = { 0x21, 0x8e, 0x8c };
    this->addColor("_mymapPos103", mymapPos103);
    int mymapPos104[3] = { 0x20, 0x8f, 0x8c };
    this->addColor("_mymapPos104", mymapPos104);
    int mymapPos105[3] = { 0x20, 0x90, 0x8c };
    this->addColor("_mymapPos105", mymapPos105);
    int mymapPos106[3] = { 0x20, 0x91, 0x8c };
    this->addColor("_mymapPos106", mymapPos106);
    int mymapPos107[3] = { 0x1f, 0x92, 0x8c };
    this->addColor("_mymapPos107", mymapPos107);
    int mymapPos108[3] = { 0x1f, 0x93, 0x8b };
    this->addColor("_mymapPos108", mymapPos108);
    int mymapPos109[3] = { 0x1f, 0x94, 0x8b };
    this->addColor("_mymapPos109", mymapPos109);
    int mymapPos110[3] = { 0x1f, 0x95, 0x8b };
    this->addColor("_mymapPos110", mymapPos110);
    int mymapPos111[3] = { 0x1f, 0x96, 0x8b };
    this->addColor("_mymapPos111", mymapPos111);
    int mymapPos112[3] = { 0x1e, 0x97, 0x8a };
    this->addColor("_mymapPos112", mymapPos112);
    int mymapPos113[3] = { 0x1e, 0x98, 0x8a };
    this->addColor("_mymapPos113", mymapPos113);
    int mymapPos114[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapPos114", mymapPos114);
    int mymapPos115[3] = { 0x1e, 0x99, 0x8a };
    this->addColor("_mymapPos115", mymapPos115);
    int mymapPos116[3] = { 0x1e, 0x9a, 0x89 };
    this->addColor("_mymapPos116", mymapPos116);
    int mymapPos117[3] = { 0x1e, 0x9b, 0x89 };
    this->addColor("_mymapPos117", mymapPos117);
    int mymapPos118[3] = { 0x1e, 0x9c, 0x89 };
    this->addColor("_mymapPos118", mymapPos118);
    int mymapPos119[3] = { 0x1e, 0x9d, 0x88 };
    this->addColor("_mymapPos119", mymapPos119);
    int mymapPos120[3] = { 0x1e, 0x9e, 0x88 };
    this->addColor("_mymapPos120", mymapPos120);
    int mymapPos121[3] = { 0x1e, 0x9f, 0x88 };
    this->addColor("_mymapPos121", mymapPos121);
    int mymapPos122[3] = { 0x1e, 0xa0, 0x87 };
    this->addColor("_mymapPos122", mymapPos122);
    int mymapPos123[3] = { 0x1f, 0xa1, 0x87 };
    this->addColor("_mymapPos123", mymapPos123);
    int mymapPos124[3] = { 0x1f, 0xa2, 0x86 };
    this->addColor("_mymapPos124", mymapPos124);
    int mymapPos125[3] = { 0x1f, 0xa3, 0x86 };
    this->addColor("_mymapPos125", mymapPos125);
    int mymapPos126[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapPos126", mymapPos126);
    int mymapPos127[3] = { 0x20, 0xa4, 0x85 };
    this->addColor("_mymapPos127", mymapPos127);
    int mymapPos128[3] = { 0x20, 0xa5, 0x85 };
    this->addColor("_mymapPos128", mymapPos128);
    int mymapPos129[3] = { 0x21, 0xa6, 0x85 };
    this->addColor("_mymapPos129", mymapPos129);
    int mymapPos130[3] = { 0x21, 0xa7, 0x84 };
    this->addColor("_mymapPos130", mymapPos130);
    int mymapPos131[3] = { 0x22, 0xa7, 0x84 };
    this->addColor("_mymapPos131", mymapPos131);
    int mymapPos132[3] = { 0x23, 0xa8, 0x83 };
    this->addColor("_mymapPos132", mymapPos132);
    int mymapPos133[3] = { 0x23, 0xa9, 0x82 };
    this->addColor("_mymapPos133", mymapPos133);
    int mymapPos134[3] = { 0x24, 0xaa, 0x82 };
    this->addColor("_mymapPos134", mymapPos134);
    int mymapPos135[3] = { 0x25, 0xab, 0x81 };
    this->addColor("_mymapPos135", mymapPos135);
    int mymapPos136[3] = { 0x26, 0xac, 0x81 };
    this->addColor("_mymapPos136", mymapPos136);
    int mymapPos137[3] = { 0x27, 0xad, 0x80 };
    this->addColor("_mymapPos137", mymapPos137);
    int mymapPos138[3] = { 0x28, 0xae, 0x7f };
    this->addColor("_mymapPos138", mymapPos138);
    int mymapPos139[3] = { 0x29, 0xaf, 0x7f };
    this->addColor("_mymapPos139", mymapPos139);
    int mymapPos140[3] = { 0x2a, 0xb0, 0x7e };
    this->addColor("_mymapPos140", mymapPos140);
    int mymapPos141[3] = { 0x2b, 0xb1, 0x7d };
    this->addColor("_mymapPos141", mymapPos141);
    int mymapPos142[3] = { 0x2c, 0xb1, 0x7d };
    this->addColor("_mymapPos142", mymapPos142);
    int mymapPos143[3] = { 0x2e, 0xb2, 0x7c };
    this->addColor("_mymapPos143", mymapPos143);
    int mymapPos144[3] = { 0x2f, 0xb3, 0x7b };
    this->addColor("_mymapPos144", mymapPos144);
    int mymapPos145[3] = { 0x30, 0xb4, 0x7a };
    this->addColor("_mymapPos145", mymapPos145);
    int mymapPos146[3] = { 0x32, 0xb5, 0x7a };
    this->addColor("_mymapPos146", mymapPos146);
    int mymapPos147[3] = { 0x33, 0xb6, 0x79 };
    this->addColor("_mymapPos147", mymapPos147);
    int mymapPos148[3] = { 0x35, 0xb7, 0x78 };
    this->addColor("_mymapPos148", mymapPos148);
    int mymapPos149[3] = { 0x36, 0xb8, 0x77 };
    this->addColor("_mymapPos149", mymapPos149);
    int mymapPos150[3] = { 0x38, 0xb9, 0x76 };
    this->addColor("_mymapPos150", mymapPos150);
    int mymapPos151[3] = { 0x39, 0xb9, 0x76 };
    this->addColor("_mymapPos151", mymapPos151);
    int mymapPos152[3] = { 0x3b, 0xba, 0x75 };
    this->addColor("_mymapPos152", mymapPos152);
    int mymapPos153[3] = { 0x3d, 0xbb, 0x74 };
    this->addColor("_mymapPos153", mymapPos153);
    int mymapPos154[3] = { 0x3e, 0xbc, 0x73 };
    this->addColor("_mymapPos154", mymapPos154);
    int mymapPos155[3] = { 0x40, 0xbd, 0x72 };
    this->addColor("_mymapPos155", mymapPos155);
    int mymapPos156[3] = { 0x42, 0xbe, 0x71 };
    this->addColor("_mymapPos156", mymapPos156);
    int mymapPos157[3] = { 0x44, 0xbe, 0x70 };
    this->addColor("_mymapPos157", mymapPos157);
    int mymapPos158[3] = { 0x45, 0xbf, 0x6f };
    this->addColor("_mymapPos158", mymapPos158);
    int mymapPos159[3] = { 0x47, 0xc0, 0x6e };
    this->addColor("_mymapPos159", mymapPos159);
    int mymapPos160[3] = { 0x49, 0xc1, 0x6d };
    this->addColor("_mymapPos160", mymapPos160);
    int mymapPos161[3] = { 0x4b, 0xc2, 0x6c };
    this->addColor("_mymapPos161", mymapPos161);
    int mymapPos162[3] = { 0x4d, 0xc2, 0x6b };
    this->addColor("_mymapPos162", mymapPos162);
    int mymapPos163[3] = { 0x4f, 0xc3, 0x69 };
    this->addColor("_mymapPos163", mymapPos163);
    int mymapPos164[3] = { 0x51, 0xc4, 0x68 };
    this->addColor("_mymapPos164", mymapPos164);
    int mymapPos165[3] = { 0x53, 0xc5, 0x67 };
    this->addColor("_mymapPos165", mymapPos165);
    int mymapPos166[3] = { 0x55, 0xc6, 0x66 };
    this->addColor("_mymapPos166", mymapPos166);
    int mymapPos167[3] = { 0x57, 0xc6, 0x65 };
    this->addColor("_mymapPos167", mymapPos167);
    int mymapPos168[3] = { 0x59, 0xc7, 0x64 };
    this->addColor("_mymapPos168", mymapPos168);
    int mymapPos169[3] = { 0x5b, 0xc8, 0x62 };
    this->addColor("_mymapPos169", mymapPos169);
    int mymapPos170[3] = { 0x5e, 0xc9, 0x61 };
    this->addColor("_mymapPos170", mymapPos170);
    int mymapPos171[3] = { 0x60, 0xc9, 0x60 };
    this->addColor("_mymapPos171", mymapPos171);
    int mymapPos172[3] = { 0x62, 0xca, 0x5f };
    this->addColor("_mymapPos172", mymapPos172);
    int mymapPos173[3] = { 0x64, 0xcb, 0x5d };
    this->addColor("_mymapPos173", mymapPos173);
    int mymapPos174[3] = { 0x67, 0xcc, 0x5c };
    this->addColor("_mymapPos174", mymapPos174);
    int mymapPos175[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapPos175", mymapPos175);
    int mymapPos176[3] = { 0x69, 0xcc, 0x5b };
    this->addColor("_mymapPos176", mymapPos176);
    int mymapPos177[3] = { 0x6b, 0xcd, 0x59 };
    this->addColor("_mymapPos177", mymapPos177);
    int mymapPos178[3] = { 0x6d, 0xce, 0x58 };
    this->addColor("_mymapPos178", mymapPos178);
    int mymapPos179[3] = { 0x70, 0xce, 0x56 };
    this->addColor("_mymapPos179", mymapPos179);
    int mymapPos180[3] = { 0x72, 0xcf, 0x55 };
    this->addColor("_mymapPos180", mymapPos180);
    int mymapPos181[3] = { 0x74, 0xd0, 0x54 };
    this->addColor("_mymapPos181", mymapPos181);
    int mymapPos182[3] = { 0x77, 0xd0, 0x52 };
    this->addColor("_mymapPos182", mymapPos182);
    int mymapPos183[3] = { 0x79, 0xd1, 0x51 };
    this->addColor("_mymapPos183", mymapPos183);
    int mymapPos184[3] = { 0x7c, 0xd2, 0x4f };
    this->addColor("_mymapPos184", mymapPos184);
    int mymapPos185[3] = { 0x7e, 0xd2, 0x4e };
    this->addColor("_mymapPos185", mymapPos185);
    int mymapPos186[3] = { 0x81, 0xd3, 0x4c };
    this->addColor("_mymapPos186", mymapPos186);
    int mymapPos187[3] = { 0x83, 0xd3, 0x4b };
    this->addColor("_mymapPos187", mymapPos187);
    int mymapPos188[3] = { 0x86, 0xd4, 0x49 };
    this->addColor("_mymapPos188", mymapPos188);
    int mymapPos189[3] = { 0x88, 0xd5, 0x47 };
    this->addColor("_mymapPos189", mymapPos189);
    int mymapPos190[3] = { 0x8b, 0xd5, 0x46 };
    this->addColor("_mymapPos190", mymapPos190);
    int mymapPos191[3] = { 0x8d, 0xd6, 0x44 };
    this->addColor("_mymapPos191", mymapPos191);
    int mymapPos192[3] = { 0x90, 0xd6, 0x43 };
    this->addColor("_mymapPos192", mymapPos192);
    int mymapPos193[3] = { 0x92, 0xd7, 0x41 };
    this->addColor("_mymapPos193", mymapPos193);
    int mymapPos194[3] = { 0x95, 0xd7, 0x3f };
    this->addColor("_mymapPos194", mymapPos194);
    int mymapPos195[3] = { 0x97, 0xd8, 0x3e };
    this->addColor("_mymapPos195", mymapPos195);
    int mymapPos196[3] = { 0x9a, 0xd8, 0x3c };
    this->addColor("_mymapPos196", mymapPos196);
    int mymapPos197[3] = { 0x9d, 0xd9, 0x3a };
    this->addColor("_mymapPos197", mymapPos197);
    int mymapPos198[3] = { 0x9f, 0xd9, 0x38 };
    this->addColor("_mymapPos198", mymapPos198);
    int mymapPos199[3] = { 0xa2, 0xda, 0x37 };
    this->addColor("_mymapPos199", mymapPos199);
    int mymapPos200[3] = { 0xa5, 0xda, 0x35 };
    this->addColor("_mymapPos200", mymapPos200);
    int mymapPos201[3] = { 0xa7, 0xdb, 0x33 };
    this->addColor("_mymapPos201", mymapPos201);
    int mymapPos202[3] = { 0xaa, 0xdb, 0x32 };
    this->addColor("_mymapPos202", mymapPos202);
    int mymapPos203[3] = { 0xad, 0xdc, 0x30 };
    this->addColor("_mymapPos203", mymapPos203);
    int mymapPos204[3] = { 0xaf, 0xdc, 0x2e };
    this->addColor("_mymapPos204", mymapPos204);
    int mymapPos205[3] = { 0xb2, 0xdd, 0x2c };
    this->addColor("_mymapPos205", mymapPos205);
    int mymapPos206[3] = { 0xb5, 0xdd, 0x2b };
    this->addColor("_mymapPos206", mymapPos206);
    int mymapPos207[3] = { 0xb7, 0xdd, 0x29 };
    this->addColor("_mymapPos207", mymapPos207);
    int mymapPos208[3] = { 0xba, 0xde, 0x27 };
    this->addColor("_mymapPos208", mymapPos208);
    int mymapPos209[3] = { 0xbd, 0xde, 0x26 };
    this->addColor("_mymapPos209", mymapPos209);
    int mymapPos210[3] = { 0xbf, 0xdf, 0x24 };
    this->addColor("_mymapPos210", mymapPos210);
    int mymapPos211[3] = { 0xc2, 0xdf, 0x22 };
    this->addColor("_mymapPos211", mymapPos211);
    int mymapPos212[3] = { 0xc5, 0xdf, 0x21 };
    this->addColor("_mymapPos212", mymapPos212);
    int mymapPos213[3] = { 0xc7, 0xe0, 0x1f };
    this->addColor("_mymapPos213", mymapPos213);
    int mymapPos214[3] = { 0xca, 0xe0, 0x1e };
    this->addColor("_mymapPos214", mymapPos214);
    int mymapPos215[3] = { 0xcd, 0xe0, 0x1d };
    this->addColor("_mymapPos215", mymapPos215);
    int mymapPos216[3] = { 0xcf, 0xe1, 0x1c };
    this->addColor("_mymapPos216", mymapPos216);
    int mymapPos217[3] = { 0xd2, 0xe1, 0x1b };
    this->addColor("_mymapPos217", mymapPos217);
    int mymapPos218[3] = { 0xd4, 0xe1, 0x1a };
    this->addColor("_mymapPos218", mymapPos218);
    int mymapPos219[3] = { 0xd7, 0xe2, 0x19 };
    this->addColor("_mymapPos219", mymapPos219);
    int mymapPos220[3] = { 0xda, 0xe2, 0x18 };
    this->addColor("_mymapPos220", mymapPos220);
    int mymapPos221[3] = { 0xdc, 0xe2, 0x18 };
    this->addColor("_mymapPos221", mymapPos221);
    int mymapPos222[3] = { 0xdf, 0xe3, 0x18 };
    this->addColor("_mymapPos222", mymapPos222);
    int mymapPos223[3] = { 0xe1, 0xe3, 0x18 };
    this->addColor("_mymapPos223", mymapPos223);
    int mymapPos224[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapPos224", mymapPos224);
    int mymapPos225[3] = { 0xe4, 0xe3, 0x18 };
    this->addColor("_mymapPos225", mymapPos225);
    int mymapPos226[3] = { 0xe7, 0xe4, 0x19 };
    this->addColor("_mymapPos226", mymapPos226);
    int mymapPos227[3] = { 0xe9, 0xe4, 0x19 };
    this->addColor("_mymapPos227", mymapPos227);
    int mymapPos228[3] = { 0xec, 0xe4, 0x1a };
    this->addColor("_mymapPos228", mymapPos228);
    int mymapPos229[3] = { 0xee, 0xe5, 0x1b };
    this->addColor("_mymapPos229", mymapPos229);
    int mymapPos230[3] = { 0xf1, 0xe5, 0x1c };
    this->addColor("_mymapPos230", mymapPos230);
    int mymapPos231[3] = { 0xfe, 0xe2, 0x90 };
    this->addColor("_mymapPos231", mymapPos231);
    int mymapPos232[3] = { 0xfe, 0xda, 0x7f };
    this->addColor("_mymapPos232", mymapPos232);
    int mymapPos233[3] = { 0xfe, 0xd3, 0x6f };
    this->addColor("_mymapPos233", mymapPos233);
    int mymapPos234[3] = { 0xfe, 0xcb, 0x5e };
    this->addColor("_mymapPos234", mymapPos234);
    int mymapPos235[3] = { 0xfe, 0xc3, 0x4e };
    this->addColor("_mymapPos235", mymapPos235);
    int mymapPos236[3] = { 0xfe, 0xb8, 0x45 };
    this->addColor("_mymapPos236", mymapPos236);
    int mymapPos237[3] = { 0xfe, 0xad, 0x3b };
    this->addColor("_mymapPos237", mymapPos237);
    int mymapPos238[3] = { 0xfe, 0xa3, 0x31 };
    this->addColor("_mymapPos238", mymapPos238);
    int mymapPos239[3] = { 0xfd, 0x98, 0x28 };
    this->addColor("_mymapPos239", mymapPos239);
    int mymapPos240[3] = { 0xf9, 0x8e, 0x23 };
    this->addColor("_mymapPos240", mymapPos240);
    int mymapPos241[3] = { 0xf4, 0x83, 0x1e };
    this->addColor("_mymapPos241", mymapPos241);
    int mymapPos242[3] = { 0xf0, 0x79, 0x18 };
    this->addColor("_mymapPos242", mymapPos242);
    int mymapPos243[3] = { 0xeb, 0x6f, 0x13 };
    this->addColor("_mymapPos243", mymapPos243);
    int mymapPos244[3] = { 0xe3, 0x66, 0xf };
    this->addColor("_mymapPos244", mymapPos244);
    int mymapPos245[3] = { 0xdb, 0x5d, 0xa };
    this->addColor("_mymapPos245", mymapPos245);
    int mymapPos246[3] = { 0xd3, 0x54, 0x6 };
    this->addColor("_mymapPos246", mymapPos246);
    int mymapPos247[3] = { 0xca, 0x4b, 0x2 };
    this->addColor("_mymapPos247", mymapPos247);
    int mymapPos248[3] = { 0xbe, 0x45, 0x2 };
    this->addColor("_mymapPos248", mymapPos248);
    int mymapPos249[3] = { 0xb1, 0x3f, 0x3 };
    this->addColor("_mymapPos249", mymapPos249);
    int mymapPos250[3] = { 0xa4, 0x39, 0x3 };
    this->addColor("_mymapPos250", mymapPos250);
    int mymapPos251[3] = { 0x97, 0x33, 0x4 };
    this->addColor("_mymapPos251", mymapPos251);
    int mymapPos252[3] = { 0x8a, 0x2f, 0x4 };
    this->addColor("_mymapPos252", mymapPos252);
    int mymapPos253[3] = { 0x7e, 0x2c, 0x5 };
    this->addColor("_mymapPos253", mymapPos253);
    int mymapPos254[3] = { 0x71, 0x28, 0x5 };
    this->addColor("_mymapPos254", mymapPos254);
    int mymapPos255[3] = { 0x66, 0x25, 0x6 };
    this->addColor("_mymapPos255", mymapPos255);
    if (this->getPaletteByName("margulies_pos") == NULL) {
        Palette mymapPos;
        mymapPos.setName("margulies_pos");
        mymapPos.addScalarAndColor( 1.000000f, "_mymapPos0");
        mymapPos.addScalarAndColor( 0.996078f, "_mymapPos1");
        mymapPos.addScalarAndColor( 0.992157f, "_mymapPos2");
        mymapPos.addScalarAndColor( 0.988235f, "_mymapPos3");
        mymapPos.addScalarAndColor( 0.984314f, "_mymapPos4");
        mymapPos.addScalarAndColor( 0.980392f, "_mymapPos5");
        mymapPos.addScalarAndColor( 0.976471f, "_mymapPos6");
        mymapPos.addScalarAndColor( 0.972549f, "_mymapPos7");
        mymapPos.addScalarAndColor( 0.968627f, "_mymapPos8");
        mymapPos.addScalarAndColor( 0.964706f, "_mymapPos9");
        mymapPos.addScalarAndColor( 0.960784f, "_mymapPos10");
        mymapPos.addScalarAndColor( 0.956863f, "_mymapPos11");
        mymapPos.addScalarAndColor( 0.952941f, "_mymapPos12");
        mymapPos.addScalarAndColor( 0.949020f, "_mymapPos13");
        mymapPos.addScalarAndColor( 0.945098f, "_mymapPos14");
        mymapPos.addScalarAndColor( 0.941176f, "_mymapPos15");
        mymapPos.addScalarAndColor( 0.937255f, "_mymapPos16");
        mymapPos.addScalarAndColor( 0.933333f, "_mymapPos17");
        mymapPos.addScalarAndColor( 0.929412f, "_mymapPos18");
        mymapPos.addScalarAndColor( 0.925490f, "_mymapPos19");
        mymapPos.addScalarAndColor( 0.921569f, "_mymapPos20");
        mymapPos.addScalarAndColor( 0.917647f, "_mymapPos21");
        mymapPos.addScalarAndColor( 0.913725f, "_mymapPos22");
        mymapPos.addScalarAndColor( 0.909804f, "_mymapPos23");
        mymapPos.addScalarAndColor( 0.905882f, "_mymapPos24");
        mymapPos.addScalarAndColor( 0.901961f, "_mymapPos25");
        mymapPos.addScalarAndColor( 0.898039f, "_mymapPos26");
        mymapPos.addScalarAndColor( 0.894118f, "_mymapPos27");
        mymapPos.addScalarAndColor( 0.890196f, "_mymapPos28");
        mymapPos.addScalarAndColor( 0.886275f, "_mymapPos29");
        mymapPos.addScalarAndColor( 0.882353f, "_mymapPos30");
        mymapPos.addScalarAndColor( 0.878431f, "_mymapPos31");
        mymapPos.addScalarAndColor( 0.874510f, "_mymapPos32");
        mymapPos.addScalarAndColor( 0.870588f, "_mymapPos33");
        mymapPos.addScalarAndColor( 0.866667f, "_mymapPos34");
        mymapPos.addScalarAndColor( 0.862745f, "_mymapPos35");
        mymapPos.addScalarAndColor( 0.858824f, "_mymapPos36");
        mymapPos.addScalarAndColor( 0.854902f, "_mymapPos37");
        mymapPos.addScalarAndColor( 0.850980f, "_mymapPos38");
        mymapPos.addScalarAndColor( 0.847059f, "_mymapPos39");
        mymapPos.addScalarAndColor( 0.843137f, "_mymapPos40");
        mymapPos.addScalarAndColor( 0.839216f, "_mymapPos41");
        mymapPos.addScalarAndColor( 0.835294f, "_mymapPos42");
        mymapPos.addScalarAndColor( 0.831373f, "_mymapPos43");
        mymapPos.addScalarAndColor( 0.827451f, "_mymapPos44");
        mymapPos.addScalarAndColor( 0.823529f, "_mymapPos45");
        mymapPos.addScalarAndColor( 0.819608f, "_mymapPos46");
        mymapPos.addScalarAndColor( 0.815686f, "_mymapPos47");
        mymapPos.addScalarAndColor( 0.811765f, "_mymapPos48");
        mymapPos.addScalarAndColor( 0.807843f, "_mymapPos49");
        mymapPos.addScalarAndColor( 0.803922f, "_mymapPos50");
        mymapPos.addScalarAndColor( 0.800000f, "_mymapPos51");
        mymapPos.addScalarAndColor( 0.796078f, "_mymapPos52");
        mymapPos.addScalarAndColor( 0.792157f, "_mymapPos53");
        mymapPos.addScalarAndColor( 0.788235f, "_mymapPos54");
        mymapPos.addScalarAndColor( 0.784314f, "_mymapPos55");
        mymapPos.addScalarAndColor( 0.780392f, "_mymapPos56");
        mymapPos.addScalarAndColor( 0.776471f, "_mymapPos57");
        mymapPos.addScalarAndColor( 0.772549f, "_mymapPos58");
        mymapPos.addScalarAndColor( 0.768627f, "_mymapPos59");
        mymapPos.addScalarAndColor( 0.764706f, "_mymapPos60");
        mymapPos.addScalarAndColor( 0.760784f, "_mymapPos61");
        mymapPos.addScalarAndColor( 0.756863f, "_mymapPos62");
        mymapPos.addScalarAndColor( 0.752941f, "_mymapPos63");
        mymapPos.addScalarAndColor( 0.749020f, "_mymapPos64");
        mymapPos.addScalarAndColor( 0.745098f, "_mymapPos65");
        mymapPos.addScalarAndColor( 0.741176f, "_mymapPos66");
        mymapPos.addScalarAndColor( 0.737255f, "_mymapPos67");
        mymapPos.addScalarAndColor( 0.733333f, "_mymapPos68");
        mymapPos.addScalarAndColor( 0.729412f, "_mymapPos69");
        mymapPos.addScalarAndColor( 0.725490f, "_mymapPos70");
        mymapPos.addScalarAndColor( 0.721569f, "_mymapPos71");
        mymapPos.addScalarAndColor( 0.717647f, "_mymapPos72");
        mymapPos.addScalarAndColor( 0.713725f, "_mymapPos73");
        mymapPos.addScalarAndColor( 0.709804f, "_mymapPos74");
        mymapPos.addScalarAndColor( 0.705882f, "_mymapPos75");
        mymapPos.addScalarAndColor( 0.701961f, "_mymapPos76");
        mymapPos.addScalarAndColor( 0.698039f, "_mymapPos77");
        mymapPos.addScalarAndColor( 0.694118f, "_mymapPos78");
        mymapPos.addScalarAndColor( 0.690196f, "_mymapPos79");
        mymapPos.addScalarAndColor( 0.686275f, "_mymapPos80");
        mymapPos.addScalarAndColor( 0.682353f, "_mymapPos81");
        mymapPos.addScalarAndColor( 0.678431f, "_mymapPos82");
        mymapPos.addScalarAndColor( 0.674510f, "_mymapPos83");
        mymapPos.addScalarAndColor( 0.670588f, "_mymapPos84");
        mymapPos.addScalarAndColor( 0.666667f, "_mymapPos85");
        mymapPos.addScalarAndColor( 0.662745f, "_mymapPos86");
        mymapPos.addScalarAndColor( 0.658824f, "_mymapPos87");
        mymapPos.addScalarAndColor( 0.654902f, "_mymapPos88");
        mymapPos.addScalarAndColor( 0.650980f, "_mymapPos89");
        mymapPos.addScalarAndColor( 0.647059f, "_mymapPos90");
        mymapPos.addScalarAndColor( 0.643137f, "_mymapPos91");
        mymapPos.addScalarAndColor( 0.639216f, "_mymapPos92");
        mymapPos.addScalarAndColor( 0.635294f, "_mymapPos93");
        mymapPos.addScalarAndColor( 0.631373f, "_mymapPos94");
        mymapPos.addScalarAndColor( 0.627451f, "_mymapPos95");
        mymapPos.addScalarAndColor( 0.623529f, "_mymapPos96");
        mymapPos.addScalarAndColor( 0.619608f, "_mymapPos97");
        mymapPos.addScalarAndColor( 0.615686f, "_mymapPos98");
        mymapPos.addScalarAndColor( 0.611765f, "_mymapPos99");
        mymapPos.addScalarAndColor( 0.607843f, "_mymapPos100");
        mymapPos.addScalarAndColor( 0.603922f, "_mymapPos101");
        mymapPos.addScalarAndColor( 0.600000f, "_mymapPos102");
        mymapPos.addScalarAndColor( 0.596078f, "_mymapPos103");
        mymapPos.addScalarAndColor( 0.592157f, "_mymapPos104");
        mymapPos.addScalarAndColor( 0.588235f, "_mymapPos105");
        mymapPos.addScalarAndColor( 0.584314f, "_mymapPos106");
        mymapPos.addScalarAndColor( 0.580392f, "_mymapPos107");
        mymapPos.addScalarAndColor( 0.576471f, "_mymapPos108");
        mymapPos.addScalarAndColor( 0.572549f, "_mymapPos109");
        mymapPos.addScalarAndColor( 0.568627f, "_mymapPos110");
        mymapPos.addScalarAndColor( 0.564706f, "_mymapPos111");
        mymapPos.addScalarAndColor( 0.560784f, "_mymapPos112");
        mymapPos.addScalarAndColor( 0.556863f, "_mymapPos113");
        mymapPos.addScalarAndColor( 0.552941f, "_mymapPos114");
        mymapPos.addScalarAndColor( 0.549020f, "_mymapPos115");
        mymapPos.addScalarAndColor( 0.545098f, "_mymapPos116");
        mymapPos.addScalarAndColor( 0.541176f, "_mymapPos117");
        mymapPos.addScalarAndColor( 0.537255f, "_mymapPos118");
        mymapPos.addScalarAndColor( 0.533333f, "_mymapPos119");
        mymapPos.addScalarAndColor( 0.529412f, "_mymapPos120");
        mymapPos.addScalarAndColor( 0.525490f, "_mymapPos121");
        mymapPos.addScalarAndColor( 0.521569f, "_mymapPos122");
        mymapPos.addScalarAndColor( 0.517647f, "_mymapPos123");
        mymapPos.addScalarAndColor( 0.513725f, "_mymapPos124");
        mymapPos.addScalarAndColor( 0.509804f, "_mymapPos125");
        mymapPos.addScalarAndColor( 0.505882f, "_mymapPos126");
        mymapPos.addScalarAndColor( 0.501961f, "_mymapPos127");
        mymapPos.addScalarAndColor( 0.498039f, "_mymapPos128");
        mymapPos.addScalarAndColor( 0.494118f, "_mymapPos129");
        mymapPos.addScalarAndColor( 0.490196f, "_mymapPos130");
        mymapPos.addScalarAndColor( 0.486275f, "_mymapPos131");
        mymapPos.addScalarAndColor( 0.482353f, "_mymapPos132");
        mymapPos.addScalarAndColor( 0.478431f, "_mymapPos133");
        mymapPos.addScalarAndColor( 0.474510f, "_mymapPos134");
        mymapPos.addScalarAndColor( 0.470588f, "_mymapPos135");
        mymapPos.addScalarAndColor( 0.466667f, "_mymapPos136");
        mymapPos.addScalarAndColor( 0.462745f, "_mymapPos137");
        mymapPos.addScalarAndColor( 0.458824f, "_mymapPos138");
        mymapPos.addScalarAndColor( 0.454902f, "_mymapPos139");
        mymapPos.addScalarAndColor( 0.450980f, "_mymapPos140");
        mymapPos.addScalarAndColor( 0.447059f, "_mymapPos141");
        mymapPos.addScalarAndColor( 0.443137f, "_mymapPos142");
        mymapPos.addScalarAndColor( 0.439216f, "_mymapPos143");
        mymapPos.addScalarAndColor( 0.435294f, "_mymapPos144");
        mymapPos.addScalarAndColor( 0.431373f, "_mymapPos145");
        mymapPos.addScalarAndColor( 0.427451f, "_mymapPos146");
        mymapPos.addScalarAndColor( 0.423529f, "_mymapPos147");
        mymapPos.addScalarAndColor( 0.419608f, "_mymapPos148");
        mymapPos.addScalarAndColor( 0.415686f, "_mymapPos149");
        mymapPos.addScalarAndColor( 0.411765f, "_mymapPos150");
        mymapPos.addScalarAndColor( 0.407843f, "_mymapPos151");
        mymapPos.addScalarAndColor( 0.403922f, "_mymapPos152");
        mymapPos.addScalarAndColor( 0.400000f, "_mymapPos153");
        mymapPos.addScalarAndColor( 0.396078f, "_mymapPos154");
        mymapPos.addScalarAndColor( 0.392157f, "_mymapPos155");
        mymapPos.addScalarAndColor( 0.388235f, "_mymapPos156");
        mymapPos.addScalarAndColor( 0.384314f, "_mymapPos157");
        mymapPos.addScalarAndColor( 0.380392f, "_mymapPos158");
        mymapPos.addScalarAndColor( 0.376471f, "_mymapPos159");
        mymapPos.addScalarAndColor( 0.372549f, "_mymapPos160");
        mymapPos.addScalarAndColor( 0.368627f, "_mymapPos161");
        mymapPos.addScalarAndColor( 0.364706f, "_mymapPos162");
        mymapPos.addScalarAndColor( 0.360784f, "_mymapPos163");
        mymapPos.addScalarAndColor( 0.356863f, "_mymapPos164");
        mymapPos.addScalarAndColor( 0.352941f, "_mymapPos165");
        mymapPos.addScalarAndColor( 0.349020f, "_mymapPos166");
        mymapPos.addScalarAndColor( 0.345098f, "_mymapPos167");
        mymapPos.addScalarAndColor( 0.341176f, "_mymapPos168");
        mymapPos.addScalarAndColor( 0.337255f, "_mymapPos169");
        mymapPos.addScalarAndColor( 0.333333f, "_mymapPos170");
        mymapPos.addScalarAndColor( 0.329412f, "_mymapPos171");
        mymapPos.addScalarAndColor( 0.325490f, "_mymapPos172");
        mymapPos.addScalarAndColor( 0.321569f, "_mymapPos173");
        mymapPos.addScalarAndColor( 0.317647f, "_mymapPos174");
        mymapPos.addScalarAndColor( 0.313725f, "_mymapPos175");
        mymapPos.addScalarAndColor( 0.309804f, "_mymapPos176");
        mymapPos.addScalarAndColor( 0.305882f, "_mymapPos177");
        mymapPos.addScalarAndColor( 0.301961f, "_mymapPos178");
        mymapPos.addScalarAndColor( 0.298039f, "_mymapPos179");
        mymapPos.addScalarAndColor( 0.294118f, "_mymapPos180");
        mymapPos.addScalarAndColor( 0.290196f, "_mymapPos181");
        mymapPos.addScalarAndColor( 0.286275f, "_mymapPos182");
        mymapPos.addScalarAndColor( 0.282353f, "_mymapPos183");
        mymapPos.addScalarAndColor( 0.278431f, "_mymapPos184");
        mymapPos.addScalarAndColor( 0.274510f, "_mymapPos185");
        mymapPos.addScalarAndColor( 0.270588f, "_mymapPos186");
        mymapPos.addScalarAndColor( 0.266667f, "_mymapPos187");
        mymapPos.addScalarAndColor( 0.262745f, "_mymapPos188");
        mymapPos.addScalarAndColor( 0.258824f, "_mymapPos189");
        mymapPos.addScalarAndColor( 0.254902f, "_mymapPos190");
        mymapPos.addScalarAndColor( 0.250980f, "_mymapPos191");
        mymapPos.addScalarAndColor( 0.247059f, "_mymapPos192");
        mymapPos.addScalarAndColor( 0.243137f, "_mymapPos193");
        mymapPos.addScalarAndColor( 0.239216f, "_mymapPos194");
        mymapPos.addScalarAndColor( 0.235294f, "_mymapPos195");
        mymapPos.addScalarAndColor( 0.231373f, "_mymapPos196");
        mymapPos.addScalarAndColor( 0.227451f, "_mymapPos197");
        mymapPos.addScalarAndColor( 0.223529f, "_mymapPos198");
        mymapPos.addScalarAndColor( 0.219608f, "_mymapPos199");
        mymapPos.addScalarAndColor( 0.215686f, "_mymapPos200");
        mymapPos.addScalarAndColor( 0.211765f, "_mymapPos201");
        mymapPos.addScalarAndColor( 0.207843f, "_mymapPos202");
        mymapPos.addScalarAndColor( 0.203922f, "_mymapPos203");
        mymapPos.addScalarAndColor( 0.200000f, "_mymapPos204");
        mymapPos.addScalarAndColor( 0.196078f, "_mymapPos205");
        mymapPos.addScalarAndColor( 0.192157f, "_mymapPos206");
        mymapPos.addScalarAndColor( 0.188235f, "_mymapPos207");
        mymapPos.addScalarAndColor( 0.184314f, "_mymapPos208");
        mymapPos.addScalarAndColor( 0.180392f, "_mymapPos209");
        mymapPos.addScalarAndColor( 0.176471f, "_mymapPos210");
        mymapPos.addScalarAndColor( 0.172549f, "_mymapPos211");
        mymapPos.addScalarAndColor( 0.168627f, "_mymapPos212");
        mymapPos.addScalarAndColor( 0.164706f, "_mymapPos213");
        mymapPos.addScalarAndColor( 0.160784f, "_mymapPos214");
        mymapPos.addScalarAndColor( 0.156863f, "_mymapPos215");
        mymapPos.addScalarAndColor( 0.152941f, "_mymapPos216");
        mymapPos.addScalarAndColor( 0.149020f, "_mymapPos217");
        mymapPos.addScalarAndColor( 0.145098f, "_mymapPos218");
        mymapPos.addScalarAndColor( 0.141176f, "_mymapPos219");
        mymapPos.addScalarAndColor( 0.137255f, "_mymapPos220");
        mymapPos.addScalarAndColor( 0.133333f, "_mymapPos221");
        mymapPos.addScalarAndColor( 0.129412f, "_mymapPos222");
        mymapPos.addScalarAndColor( 0.125490f, "_mymapPos223");
        mymapPos.addScalarAndColor( 0.121569f, "_mymapPos224");
        mymapPos.addScalarAndColor( 0.117647f, "_mymapPos225");
        mymapPos.addScalarAndColor( 0.113725f, "_mymapPos226");
        mymapPos.addScalarAndColor( 0.109804f, "_mymapPos227");
        mymapPos.addScalarAndColor( 0.105882f, "_mymapPos228");
        mymapPos.addScalarAndColor( 0.101961f, "_mymapPos229");
        mymapPos.addScalarAndColor( 0.098039f, "_mymapPos230");
        mymapPos.addScalarAndColor( 0.094118f, "_mymapPos231");
        mymapPos.addScalarAndColor( 0.090196f, "_mymapPos232");
        mymapPos.addScalarAndColor( 0.086275f, "_mymapPos233");
        mymapPos.addScalarAndColor( 0.082353f, "_mymapPos234");
        mymapPos.addScalarAndColor( 0.078431f, "_mymapPos235");
        mymapPos.addScalarAndColor( 0.074510f, "_mymapPos236");
        mymapPos.addScalarAndColor( 0.070588f, "_mymapPos237");
        mymapPos.addScalarAndColor( 0.066667f, "_mymapPos238");
        mymapPos.addScalarAndColor( 0.062745f, "_mymapPos239");
        mymapPos.addScalarAndColor( 0.058824f, "_mymapPos240");
        mymapPos.addScalarAndColor( 0.054902f, "_mymapPos241");
        mymapPos.addScalarAndColor( 0.050980f, "_mymapPos242");
        mymapPos.addScalarAndColor( 0.047059f, "_mymapPos243");
        mymapPos.addScalarAndColor( 0.043137f, "_mymapPos244");
        mymapPos.addScalarAndColor( 0.039216f, "_mymapPos245");
        mymapPos.addScalarAndColor( 0.035294f, "_mymapPos246");
        mymapPos.addScalarAndColor( 0.031373f, "_mymapPos247");
        mymapPos.addScalarAndColor( 0.027451f, "_mymapPos248");
        mymapPos.addScalarAndColor( 0.023529f, "_mymapPos249");
        mymapPos.addScalarAndColor( 0.019608f, "_mymapPos250");
        mymapPos.addScalarAndColor( 0.015686f, "_mymapPos251");
        mymapPos.addScalarAndColor( 0.011765f, "_mymapPos252");
        mymapPos.addScalarAndColor( 0.007843f, "_mymapPos253");
        mymapPos.addScalarAndColor( 0.003922f, "_mymapPos254");
        mymapPos.addScalarAndColor( 0.000000f, "_mymapPos255");
        addPalette(mymapPos);
    }

    //------------------------------------------------------------------------
    //
    // Palette by Jon Wieser @ mcw
    //
    int rbgyr20_01[3] = { 0xCC, 0x10, 0x33 };
    this->addColor("_rbgyr20_01", rbgyr20_01);
    int rbgyr20_02[3] = { 0x99, 0x20, 0x66 };
    this->addColor("_rbgyr20_02", rbgyr20_02);
    int rbgyr20_03[3] = { 0x66, 0x31, 0x99 };
    this->addColor("_rbgyr20_03", rbgyr20_03);
    int rbgyr20_04[3] = { 0x34, 0x41, 0xCC };
    this->addColor("_rbgyr20_04", rbgyr20_04);
    int rbgyr20_05[3] = { 0x00, 0x51, 0xFF };
    this->addColor("_rbgyr20_05", rbgyr20_05);
    int rbgyr20_06[3] = { 0x00, 0x74, 0xCC };
    this->addColor("_rbgyr20_06", rbgyr20_06);
    int rbgyr20_07[3] = { 0x00, 0x97, 0x99 };
    this->addColor("_rbgyr20_07", rbgyr20_07);
    int rbgyr20_08[3] = { 0x00, 0xB9, 0x66 };
    this->addColor("_rbgyr20_08", rbgyr20_08);
    int rbgyr20_09[3] = { 0x00, 0xDC, 0x33 };
    this->addColor("_rbgyr20_09", rbgyr20_09);
    int rbgyr20_10[3] = { 0x00, 0xFF, 0x00 };
    this->addColor("_rbgyr20_10", rbgyr20_10);
    int rbgyr20_11[3] = { 0x33, 0xFF, 0x00 };
    this->addColor("_rbgyr20_11", rbgyr20_11);
    int rbgyr20_12[3] = { 0x66, 0xFF, 0x00 };
    this->addColor("_rbgyr20_12", rbgyr20_12);
    int rbgyr20_13[3] = { 0x99, 0xFF, 0x00 };
    this->addColor("_rbgyr20_13", rbgyr20_13);
    int rbgyr20_14[3] = { 0xCC, 0xFF, 0x00 };
    this->addColor("_rbgyr20_14", rbgyr20_14);
    int rbgyr20_15[3] = { 0xFF, 0xFF, 0x00 };
    this->addColor("_rbgyr20_15", rbgyr20_15);
    int rbgyr20_16[3] = { 0xFF, 0xCC, 0x00 };
    this->addColor("_rbgyr20_16", rbgyr20_16);
    int rbgyr20_17[3] = { 0xFF, 0x99, 0x00 };
    this->addColor("_rbgyr20_17", rbgyr20_17);
    int rbgyr20_18[3] = { 0xFF, 0x66, 0x00 };
    this->addColor("_rbgyr20_18", rbgyr20_18);
    int rbgyr20_19[3] = { 0xFF, 0x33, 0x00 };
    this->addColor("_rbgyr20_19", rbgyr20_19);
    int rbgyr20_20[3] = { 0xFF, 0x00, 0x00 };
    this->addColor("_rbgyr20_20", rbgyr20_20);

    if (this->getPaletteByName("RBGYR20") == NULL) {
        Palette pal2;
        pal2.setName("RBGYR20");
        pal2.addScalarAndColor( 1.0f, "_rbgyr20_01");
        pal2.addScalarAndColor( 0.9f, "_rbgyr20_02");
        pal2.addScalarAndColor( 0.8f, "_rbgyr20_03");
        pal2.addScalarAndColor( 0.7f, "_rbgyr20_04");
        pal2.addScalarAndColor( 0.6f, "_rbgyr20_05");
        pal2.addScalarAndColor( 0.5f, "_rbgyr20_06");
        pal2.addScalarAndColor( 0.4f, "_rbgyr20_07");
        pal2.addScalarAndColor( 0.3f, "_rbgyr20_08");
        pal2.addScalarAndColor( 0.2f, "_rbgyr20_09");
        pal2.addScalarAndColor( 0.1f, "_rbgyr20_10");
        pal2.addScalarAndColor( 0.0f, "_rbgyr20_11");
        pal2.addScalarAndColor(-0.1f, "_rbgyr20_12");
        pal2.addScalarAndColor(-0.2f, "_rbgyr20_13");
        pal2.addScalarAndColor(-0.3f, "_rbgyr20_14");
        pal2.addScalarAndColor(-0.4f, "_rbgyr20_15");
        pal2.addScalarAndColor(-0.5f, "_rbgyr20_16");
        pal2.addScalarAndColor(-0.6f, "_rbgyr20_17");
        pal2.addScalarAndColor(-0.7f, "_rbgyr20_18");
        pal2.addScalarAndColor(-0.8f, "_rbgyr20_19");
        pal2.addScalarAndColor(-0.9f, "_rbgyr20_20");
        addPalette(pal2);

        Palette pal3;
        pal3.setName("RBGYR20P");
        pal3.addScalarAndColor(1.00f, "_rbgyr20_01");
        pal3.addScalarAndColor(0.95f, "_rbgyr20_02");
        pal3.addScalarAndColor(0.90f, "_rbgyr20_03");
        pal3.addScalarAndColor(0.85f, "_rbgyr20_04");
        pal3.addScalarAndColor(0.80f, "_rbgyr20_05");
        pal3.addScalarAndColor(0.75f, "_rbgyr20_06");
        pal3.addScalarAndColor(0.70f, "_rbgyr20_07");
        pal3.addScalarAndColor(0.65f, "_rbgyr20_08");
        pal3.addScalarAndColor(0.60f, "_rbgyr20_09");
        pal3.addScalarAndColor(0.55f, "_rbgyr20_10");
        pal3.addScalarAndColor(0.50f, "_rbgyr20_11");
        pal3.addScalarAndColor(0.45f, "_rbgyr20_12");
        pal3.addScalarAndColor(0.40f, "_rbgyr20_13");
        pal3.addScalarAndColor(0.35f, "_rbgyr20_14");
        pal3.addScalarAndColor(0.30f, "_rbgyr20_15");
        pal3.addScalarAndColor(0.25f, "_rbgyr20_16");
        pal3.addScalarAndColor(0.20f, "_rbgyr20_17");
        pal3.addScalarAndColor(0.15f, "_rbgyr20_18");
        pal3.addScalarAndColor(0.10f, "_rbgyr20_19");
        pal3.addScalarAndColor(0.05f, "_rbgyr20_20");
        pal3.addScalarAndColor(0.0f, "none");
        addPalette(pal3);
    }

    //----------------------------------------------------------------------
    // Positive/Negative palette
    //
    if (this->getPaletteByName("POS_NEG") == NULL) {
        this->addColor("pos_neg_blue",  0x00, 0x00, 0xff );
        this->addColor("pos_neg_red",  0xff, 0x00, 0x00 );

        Palette posNeg;
        posNeg.setName("POS_NEG");

        posNeg.addScalarAndColor(1.00f, "pos_neg_red");
        posNeg.addScalarAndColor(0.0001f, "none");
        posNeg.addScalarAndColor(-0.0001f, "pos_neg_blue");

        addPalette(posNeg);
    }

    if (this->getPaletteByName("red-yellow") == NULL) {
        this->addColor("_red_yellow_interp_red",  255, 0, 0 );
        this->addColor("_red_yellow_interp_yellow",  255, 255, 0 );
        this->addColor("_blue_lightblue_interp_blue",  0, 0, 255 );
        this->addColor("_blue_lightblue_interp_lightblue",  0, 255, 255 );
        this->addColor("_fslview_zero", 0, 0, 0);

        Palette palRedYellowInterp;
        palRedYellowInterp.setName("red-yellow");
        palRedYellowInterp.addScalarAndColor(1.0f, "_red_yellow_interp_yellow");
        palRedYellowInterp.addScalarAndColor(0.0f, "_red_yellow_interp_red");
        addPalette(palRedYellowInterp);

        Palette palBlueLightblueInterp;
        palBlueLightblueInterp.setName("blue-lightblue");
        palBlueLightblueInterp.addScalarAndColor(1.0f, "_blue_lightblue_interp_lightblue");
        palBlueLightblueInterp.addScalarAndColor(0.0f, "_blue_lightblue_interp_blue");
        addPalette(palBlueLightblueInterp);

        Palette palFSLView;
        palFSLView.setName("FSL");
        palFSLView.addScalarAndColor( 1.0f, "_red_yellow_interp_yellow");
        palFSLView.addScalarAndColor( 0.00001f, "_red_yellow_interp_red");
        palFSLView.addScalarAndColor( 0.0000099f, "_fslview_zero");
        palFSLView.addScalarAndColor(-0.0000099f, "_fslview_zero");
        palFSLView.addScalarAndColor(-0.00001f, "_blue_lightblue_interp_blue");
        palFSLView.addScalarAndColor(-1.0f, "_blue_lightblue_interp_lightblue");
        addPalette(palFSLView);
    }

    if (this->getPaletteByName("power_surf") == NULL) {
        this->addColor("_ps_0",    1.0 *255.0,   0.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_059",  0.0 * 255.0,  0.0 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_118",  1.0 * 255.0,  1.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_176",  1.0 * 255.0,  0.7 * 255.0,  0.4 * 255.0);
        this->addColor("_ps_235",  0.0 * 255.0,  0.8 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_294",  1.0 * 255.0,  0.6 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_353",  0.0 * 255.0,  0.6 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_412",  0.0 * 255.0,  0.0 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_471",  0.3 * 255.0,  0.0 * 255.0,  0.6 * 255.0 );
        this->addColor("_ps_529",  0.2 * 255.0,  1.0 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_588",  1.0 * 255.0,  0.5 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_647",  0.6 * 255.0,  0.2 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_706",  0.0 * 255.0,  0.2 * 255.0,  0.4 * 255.0 );
        this->addColor("_ps_765",  0.2 * 255.0,  1.0 * 255.0,  0.2 * 255.0 );
        this->addColor("_ps_824",  0.0 * 255.0,  0.0 * 255.0,  1.0 * 255.0 );
        this->addColor("_ps_882",  1.0 * 255.0,  1.0 * 255.0,  0.8 * 255.0 );
        this->addColor("_ps_941",  0.0 * 255.0,  0.4 * 255.0,  0.0 * 255.0 );
        this->addColor("_ps_1000", 0.25 * 255.0, 0.25 * 255.0, 0.25 * 255.0 );

        Palette powerSurf;
        powerSurf.setName("power_surf");
        powerSurf.addScalarAndColor( 1.0, "_ps_1000");
        powerSurf.addScalarAndColor( 0.941, "_ps_941");
        powerSurf.addScalarAndColor( 0.882, "_ps_882");
        powerSurf.addScalarAndColor( 0.824, "_ps_824");
        powerSurf.addScalarAndColor( 0.765, "_ps_765");
        powerSurf.addScalarAndColor( 0.706, "_ps_706");
        powerSurf.addScalarAndColor( 0.647, "_ps_647");
        powerSurf.addScalarAndColor( 0.588, "_ps_588");
        powerSurf.addScalarAndColor( 0.529, "_ps_529");
        powerSurf.addScalarAndColor( 0.471, "_ps_471");
        powerSurf.addScalarAndColor( 0.412, "_ps_412");
        powerSurf.addScalarAndColor( 0.353, "_ps_353");
        powerSurf.addScalarAndColor( 0.294, "_ps_294");
        powerSurf.addScalarAndColor( 0.235, "_ps_235");
        powerSurf.addScalarAndColor( 0.176, "_ps_176");
        powerSurf.addScalarAndColor( 0.118, "_ps_118");
        powerSurf.addScalarAndColor( 0.059, "_ps_059");
        powerSurf.addScalarAndColor( 0.0, "_ps_0");
        addPalette(powerSurf);
    }

    /*
     * FSL Red palette from WB-289
     *
     * float offset = 100.0;
     * float step = (255.0 - offset) / 255.0;
     * for(unsigned char i = 0; i < 255; ++i)
     * { int red = int(((i + 1) * step) + offset); lut->pushValue(red, 0, 0, i); }
     *
     * lut->m_lutName = std::string("Red");
     */
    //TSC: no "lookup tables" for purely interpolated palettes!  bad for performance.
    if (this->getPaletteByName("fsl_red") == NULL) {
        Palette fslRed;
        fslRed.setName("fsl_red");
        this->addColor("fsl_red_0", 100, 0, 0);
        this->addColor("fsl_red_1", 255, 0, 0);
        fslRed.addScalarAndColor(1.0f, "fsl_red_1");
        fslRed.addScalarAndColor(0.0f, "fsl_red_0");
        addPalette(fslRed);
    }

    if (this->getPaletteByName("fsl_green") == NULL) {
        Palette fslGreen;
        fslGreen.setName("fsl_green");
        this->addColor("fsl_green_0", 0, 100, 0);
        this->addColor("fsl_green_1", 0, 255, 0);
        fslGreen.addScalarAndColor(1.0f, "fsl_green_1");
        fslGreen.addScalarAndColor(0.0f, "fsl_green_0");
        addPalette(fslGreen);
    }

    if (this->getPaletteByName("fsl_blue") == NULL) {
        Palette fslBlue;
        fslBlue.setName("fsl_blue");
        this->addColor("fsl_blue_0", 0, 0, 100);
        this->addColor("fsl_blue_1", 0, 0, 255);
        fslBlue.addScalarAndColor(1.0f, "fsl_blue_1");
        fslBlue.addScalarAndColor(0.0f, "fsl_blue_0");
        addPalette(fslBlue);
    }

    if (this->getPaletteByName("fsl_yellow") == NULL) {
        Palette fslYellow;
        fslYellow.setName("fsl_yellow");
        this->addColor("fsl_yellow_0", 100, 100, 0);
        this->addColor("fsl_yellow_1", 255, 255, 0);
        fslYellow.addScalarAndColor(1.0f, "fsl_yellow_1");
        fslYellow.addScalarAndColor(0.0f, "fsl_yellow_0");
        addPalette(fslYellow);
    }

    if (this->getPaletteByName("JET256") == NULL) {
        Palette JET256;
        JET256.setName("JET256");

        //summary of original slow "lookup table" (if closer to previous implementation is desired):
        //start: 0 -> (0 0 132)
        //change: 0.121 -> (0 0 255)
        //change: 0.372 -> (0 255 255)
        //change: 0.623 -> (255 255 0)
        //change: 0.874 -> (255 0 0)
        //end: 1 -> (127 0 0)

        //alternative round-valued version via https://gist.github.com/bagrow/805122
        /*(0 0.0 0.0 0.5, \
           1 0.0 0.0 1.0, \
           2 0.0 0.5 1.0, \ <- redundant
           3 0.0 1.0 1.0, \
           4 0.5 1.0 0.5, \ <- redundant
           5 1.0 1.0 0.0, \
           6 1.0 0.5 0.0, \ <- redundant
           7 1.0 0.0 0.0, \
           8 0.5 0.0 0.0 )*/

        this->addColor("_J0", 0, 0, 127);//rounding to probably-intended colors
        this->addColor("_J1", 0, 0, 255);
        this->addColor("_J3", 0, 255, 255);//skipping redundant points
        this->addColor("_J5", 255, 255, 0);
        this->addColor("_J7", 255, 0, 0);
        this->addColor("_J8", 127, 0, 0);

        JET256.addScalarAndColor(1.0f, "_J8");
        JET256.addScalarAndColor(0.875f, "_J7");//also rounding to probably-intended control points
        JET256.addScalarAndColor(0.625f, "_J5");
        JET256.addScalarAndColor(0.375f, "_J3");
        JET256.addScalarAndColor(0.125f, "_J1");
        JET256.addScalarAndColor(0.0f, "_J0");

        addPalette(JET256);
    }
    if (modifiedStatus == false) {
        this->clearModified();
    }
}

/**
 * @return The structure for this file.
 */
StructureEnum::Enum
PaletteFile::getStructure() const
{
    // palette files do not have structure
    return StructureEnum::INVALID;
}

/**
 * Set the structure for this file.
 * @param structure
 *   New structure for this file.
 */
void
PaletteFile::setStructure(const StructureEnum::Enum /*structure*/)
{
    // palette files do not have structure
}

/**
 * @return Get access to the file's metadata.
 */
GiftiMetaData*
PaletteFile::getFileMetaData()
{
    return this->metadata;
}

/**
 * @return Get access to unmodifiable file's metadata.
 */
const GiftiMetaData*
PaletteFile::getFileMetaData() const
{
    return this->metadata;
}

/**
 * Set the palette mapping based upon the given file type,
 * file name, data name, and data.
 *
 * @param paletteColorMapping
 *    Palette color mapping that is setup.
 * @param dataFileType
 *    Type of data file.
 * @param fileName
 *    Name of file.
 * @param dataName
 *    Name of data.
 * @param data
 *    The data.
 * @param numberOfDataElements
 *    Number of elements in data.
 */
void
PaletteFile::setDefaultPaletteColorMapping(PaletteColorMapping* paletteColorMapping,
                                           const DataFileTypeEnum::Enum& dataFileType,
                                           const AString& fileNameIn,
                                           const AString& dataNameIn,
                                           const float* data,
                                           const int32_t numberOfDataElements)
{
    bool isShapeCurvatureData = false;
    bool isShapeDepthData = false;
    bool isShapeData = false;
    bool isVolumeAnatomyData = false;

    const AString fileName = fileNameIn.toLower();
    const AString dataName = dataNameIn.toLower();

    bool invalid = false;
    bool checkShapeFile = false;
    bool checkVolume = false;
    switch (dataFileType) {
        case DataFileTypeEnum::ANNOTATION:
            invalid = true;
            break;
        case DataFileTypeEnum::BORDER:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_DYNAMIC:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_LABEL:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_PARCEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_DENSE:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_LABEL:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_SCALAR:
            break;
        case DataFileTypeEnum::CONNECTIVITY_PARCEL_SERIES:
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_SCALAR:
            checkShapeFile = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_DENSE_TIME_SERIES:
            break;
        case DataFileTypeEnum::CONNECTIVITY_FIBER_ORIENTATIONS_TEMPORARY:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_FIBER_TRAJECTORY_TEMPORARY:
            invalid = true;
            break;
        case DataFileTypeEnum::CONNECTIVITY_SCALAR_DATA_SERIES:
            break;
        case DataFileTypeEnum::FOCI:
            invalid = true;
            break;
        case DataFileTypeEnum::IMAGE:
            invalid = true;
            break;
        case DataFileTypeEnum::LABEL:
            invalid = true;
            break;
        case DataFileTypeEnum::METRIC:
            checkShapeFile = true;
            break;
        case DataFileTypeEnum::PALETTE:
            invalid = true;
            break;
        case DataFileTypeEnum::RGBA:
            invalid = true;
            break;
        case DataFileTypeEnum::SCENE:
            invalid = true;
            break;
        case DataFileTypeEnum::SPECIFICATION:
            invalid = true;
            break;
        case DataFileTypeEnum::SURFACE:
            invalid = true;
            break;
        case DataFileTypeEnum::UNKNOWN:
            invalid = true;
            break;
        case DataFileTypeEnum::VOLUME:
            checkVolume = true;
            break;
    }

    if (invalid) {
        return;
    }

    if (checkShapeFile) {
        if (dataName.contains("curv")) {
            isShapeData = true;
            isShapeCurvatureData = true;
        }
        else if (dataName.contains("depth")) {
            isShapeData = true;
            isShapeDepthData = true;
        }
        else if (dataName.contains("shape")) {
            isShapeData = true;
        }
        else if (fileName.contains("curv")) {
            isShapeData = true;
            isShapeCurvatureData = true;
        }
        else if (fileName.contains("depth")) {
            isShapeData = true;
            isShapeDepthData = true;
        }
        else if (fileName.contains("shape")) {
            isShapeData = true;
        }
    }

    float minValue = std::numeric_limits<float>::max();
    float maxValue = -minValue;

    for (int32_t i = 0; i < numberOfDataElements; i++) {
        const float d = data[i];
        if (d > maxValue) {
            maxValue = d;
        }
        if (d < minValue) {
            minValue = d;
        }
    }
    //bool havePositiveData = (maxValue > 0);//unused, commenting out to prevent compiler warning
    bool haveNegativeData = (minValue < 0);

    if (checkVolume) {
        if ((minValue >= 0)
            && (maxValue <= 255.0)) {
            isVolumeAnatomyData = true;
        }
    }

    if (isVolumeAnatomyData) {
        paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
        paletteColorMapping->setSelectedPaletteName("Gray_Interp_Positive");
        paletteColorMapping->setInterpolatePaletteFlag(true);
        paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
        paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
        paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
        paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
        paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
    }
    else if (isShapeData) {
        paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
        paletteColorMapping->setSelectedPaletteName("Gray_Interp");
        paletteColorMapping->setInterpolatePaletteFlag(true);
        if (isShapeDepthData) {
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_USER_SCALE);
            paletteColorMapping->setUserScaleNegativeMaximum(-30.0);
            paletteColorMapping->setUserScaleNegativeMinimum(0.0);
            paletteColorMapping->setUserScalePositiveMinimum(0.0);
            paletteColorMapping->setUserScalePositiveMaximum(10.0);
        }
        else if (isShapeCurvatureData) {
//            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_USER_SCALE);
//            paletteColorMapping->setUserScaleNegativeMaximum(-1.5);
//            paletteColorMapping->setUserScaleNegativeMinimum(0.0);
//            paletteColorMapping->setUserScalePositiveMinimum(0.0);
//            paletteColorMapping->setUserScalePositiveMaximum(1.5);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
        }
        else {
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
        }
        paletteColorMapping->setDisplayNegativeDataFlag(true);
        paletteColorMapping->setDisplayPositiveDataFlag(true);
        paletteColorMapping->setDisplayZeroDataFlag(true);
    }
    else {
        if (haveNegativeData) {
            paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
            paletteColorMapping->setSelectedPaletteName("videen-style");
            paletteColorMapping->setSelectedPaletteName("ROY-BIG-BL");
            paletteColorMapping->setInterpolatePaletteFlag(true);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
            paletteColorMapping->setDisplayNegativeDataFlag(true);
            paletteColorMapping->setDisplayPositiveDataFlag(true);
            paletteColorMapping->setDisplayZeroDataFlag(false);
        }
        else {
            paletteColorMapping->setThresholdType(PaletteThresholdTypeEnum::THRESHOLD_TYPE_OFF);
            paletteColorMapping->setSelectedPaletteName("videen-style");
            paletteColorMapping->setSelectedPaletteName("ROY-BIG-BL");
            paletteColorMapping->setInterpolatePaletteFlag(true);
            paletteColorMapping->setScaleMode(PaletteScaleModeEnum::MODE_AUTO_SCALE_PERCENTAGE);
            paletteColorMapping->setAutoScalePercentageNegativeMaximum(98.0);
            paletteColorMapping->setAutoScalePercentageNegativeMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMinimum(2.0);
            paletteColorMapping->setAutoScalePercentagePositiveMaximum(98.0);
            paletteColorMapping->setDisplayNegativeDataFlag(true);
            paletteColorMapping->setDisplayPositiveDataFlag(true);
            paletteColorMapping->setDisplayZeroDataFlag(false);
        }
    }
    paletteColorMapping->clearModified();
}
