
#ifndef __SCENE_INFO_SAX_READER_H__
#define __SCENE_INFO_SAX_READER_H__

/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/

#include <stack>
#include <stdint.h>

#include "AString.h"
#include "XmlSaxParserException.h"
#include "XmlSaxParserHandlerInterface.h"


namespace caret {

    class SceneInfo;
    class XmlAttributes;
    
    /**
     * class for reading a Scene Info with a SAX Parser
     */
    class SceneInfoSaxReader : public CaretObject, public XmlSaxParserHandlerInterface {
    public:
        SceneInfoSaxReader(const AString& sceneFileName,
                       SceneInfo* sceneInfo);
        
        virtual ~SceneInfoSaxReader();
        
        void startElement(const AString& namespaceURI,
                          const AString& localName,
                          const AString& qName,
                          const XmlAttributes& attributes) throw (XmlSaxParserException);
        
        void endElement(const AString& namspaceURI,
                        const AString& localName,
                        const AString& qName) throw (XmlSaxParserException);
        
        void characters(const char* ch) throw (XmlSaxParserException);
        
        void fatalError(const XmlSaxParserException& e) throw (XmlSaxParserException);
        
        void warning(const XmlSaxParserException& e) throw (XmlSaxParserException);
        
        void error(const XmlSaxParserException& e) throw (XmlSaxParserException);
        
        void startDocument() throw (XmlSaxParserException);
        
        void endDocument() throw (XmlSaxParserException);
        
    protected:
        /// file reading states
        enum STATE {
            /// no state
            STATE_NONE,
            /// processing Scene Info tag
            STATE_SCENE_INFO,
            /// processing Scene Info Name tag
            STATE_SCENE_INFO_NAME,
            /// processing Scene Info Description tag
            STATE_SCENE_INFO_DESCRIPTION,
            /// processing Scene Info thumbnail tag
            STATE_SCENE_INFO_IMAGE_THUMBNAIL
        };
        
        /// name of scene file
        AString m_sceneFileName;
        
        /// file reading state
        STATE m_state;
        
        /// the state stack used when reading a file
        std::stack<STATE> m_stateStack;
        
        /// the error message
        AString m_errorMessage;
        
        /// element text
        AString m_elementText;
        
        /// Scene being read
        SceneInfo* m_sceneInfo;
        
        /** Index of scene info */
        int32_t m_sceneInfoIndex;
       
        /** image format */
        AString m_imageFormat;
        
        /** image encoding */
        AString m_imageEncoding;
    };

} // namespace

#endif // __SCENE_INFO_SAX_READER_H__
