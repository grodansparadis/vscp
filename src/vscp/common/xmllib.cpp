/*---------------------------------------------------------------------------
** Project Name: VSCP
** #Module: # XML library for QT
** #Author: # Dinesh Guleria
** #Date: # 15-03-2013
** (c) Copyright 2013,
**----------------------------------------------------------------------------
** #Revision: 1 #
**----------------------------------------------------------------------------
** Target system: QT
** Compiler:
**----------------------------------------------------------------------------
**               A U T H O R   I D E N T I T Y
**----------------------------------------------------------------------------
** Initials           Name                     Company
** --------           ---------------------    -------------------------------
** DINESH.GULERIA     Dinesh Guleria           Ignite Solutions India
**----------------------------------------------------------------------------
**               R E V I S I O N   H I S T O R Y
**----------------------------------------------------------------------------
** #Log:
** 1. Creation of the file.
** 2. Review the implementation to bring it to 100%.
**----------------------------------------------------------------------------
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**--------------------------------------------------------------------------*/

#include "xmlLib.h"
#include <QMessageBox>


xmlLib::xmlLib()
{
    //Do nothing
}


xmlLib::~xmlLib()
{
    //Do nothing
}

//************** file ********************************

int xmlLib::xmlOpenFile( QString &xmlFileName, QFile &xmlFile, QTextStream &xmlStream)
{
    //set the name of the file
    xmlFile.setFileName(xmlFileName);

    // open read & write mode
    if (!xmlFile.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        return FAIL_TO_OPEN_FILE;
    }

    // Assign file to the stream
    xmlStream.setDevice(&xmlFile);

    return SUCCESS_TO_OPEN_FILE;
}

int xmlLib::xmlOpenFileTruncate( QString &xmlFileName, QFile &xmlFile, QTextStream &xmlStream)
{
    //set the name of the file
    xmlFile.setFileName(xmlFileName);

    // open read & write mode
    if (!xmlFile.open(QIODevice::ReadWrite|QIODevice::Truncate|QIODevice::Text))
    {
        return FAIL_TO_OPEN_FILE;
    }

    // Assign file to the stream
    xmlStream.setDevice(&xmlFile);

    return SUCCESS_TO_OPEN_FILE;
}


//Load the XML file
void xmlLib::xmlLoadFile(QDomDocument &xmlDomDocument, QFile &xmlFile)
{
    xmlDomDocument.clear();
    xmlDomDocument.setContent(&xmlFile);
}

//close file
void xmlLib::xmlCloseFile(QTextStream &xmlStream, QDomDocument &xmlDomDocument, QFile &xmlFile)
{
    // Write xml to the file
    (xmlStream) << xmlDomDocument.toString();

    // close the file
    xmlFile.flush();
    xmlFile.close();
}

//close file no flush
void xmlLib::xmlCloseFileNoFlush(QFile &xmlFile)
{
    // Write xml to the file
    //(xmlStream) << xmlDomDocument.toString();

    // close the file
    //xmlFile.flush();
    xmlFile.close();
}
//********************* Node creation *************************

//Add the root item
void xmlLib::xmlAddRoot(QDomElement &xmlRoot, QDomDocument &xmlDomDocument)
{
    // Make the root element
    xmlRoot = xmlDomDocument.createElement(ROOT_ELEMENT_DETAILS);

    // Add it to the document
    xmlDomDocument.appendChild(xmlRoot);
}

//Get the root item
void xmlLib::xmlGetRoot(QDomElement &xmlRoot, QDomDocument &xmlDomDocument)
{
    QMessageBox* msgBox_2;
    // Make the root element
    xmlRoot = xmlDomDocument.documentElement();

    if(xmlRoot.isNull()){
        msgBox_2 = new QMessageBox();
        msgBox_2->setWindowTitle("Confirmation");
        msgBox_2->setText(XML_ROOTNULL_MESSAGE);
        msgBox_2->exec();
   }
    // Add it to the document
    //xmlDomDocument.appendChild(xmlRoot);
}

// Create the node
void xmlLib::xmlCreateNode(QDomDocument &xmlDomDocument, QDomElement &NodeElement, QString Name)
{
    // Make the node element
    NodeElement = xmlDomDocument.createElement(Name);

}

// return number of direct root childs
int xmlLib::xmlRootDirectChildNode(QDomDocument &xmlDomDocument)
{
    // return number of direct root childs
       //xmlRoot --> xmlDomDocument.documentElement()
    return xmlDomDocument.documentElement().childNodes().count();
}

// find number of elements attached to the node
int xmlLib::xmlTotalChildNode(QDomNode &NodeElement)
{
    if(!NodeElement.firstChild().isNull())
    {
        return NodeElement.childNodes().count();
    }
    else
    {
        // Node is NULL
        return 0;
    }
}


// Reading particular child node from the parent
void xmlLib::xmlGetNodeByNumber(QDomNode Parent, int nodeNumber, QDomNode &temp)
{
    QDomNodeList list;
    if(!Parent.firstChild().isNull())
    {
        list = Parent.childNodes();
        temp = list.at(nodeNumber).toElement();
    }
    else
    {
        // Do nothing
    }
}
// Create the text node
void xmlLib::xmlAddTextNode(QDomDocument &xmlDomDocument, QDomElement &NodeElement, QString textContent)
{
    QDomText textNode;
    textNode = xmlDomDocument.createTextNode(textContent);
    NodeElement.appendChild(textNode);
}

//Append node to the parent
void xmlLib::xmlAppendNode(QDomElement &xmlParent, QDomElement &xmlChild)
{
    xmlParent.appendChild(xmlChild);

}
// Replace old node with new one
void xmlLib::xmlUpdateNode(QDomElement &xmlRoot, QDomNode &newNode,QDomNode &oldNode)
{
    // replace existing node with new node
    xmlRoot.replaceChild(newNode, oldNode);
}

// Remove child from xml Root -- i.e -- remove test case
void xmlLib::xmlRootDeleteNode(QDomElement &xmlRoot, QDomNode &nodeToRemove)
{
    if(!nodeToRemove.isNull())
    {
        // Remove node from the xml root
        xmlRoot.removeChild(nodeToRemove);
    }

}

// ******************** Node values *****************

// Set the value of the xml node
void xmlLib::xmlWriteNodeValue(QDomElement &NodeElement, QString NodeValueTxt)
{
    NodeElement.setNodeValue(NodeValueTxt);
}

// Read the value of the xml node
void xmlLib::xmlReadNodeValue(QDomNode &NodeToReadValue, QString NodeValueTxt)
{
    NodeValueTxt = NodeToReadValue.nodeValue();
}


// Reading particular child node from the parent
void xmlLib::xmlRootRemoveNodeByNumber(QDomElement &xmlRoot, int nodeNumber)
{
    QDomNodeList list;
    QDomNode temp;

    if(!xmlRoot.firstChild().isNull())
    {
        list = xmlRoot.childNodes();
        temp = list.at(nodeNumber);

        xmlRoot.removeChild(temp);
    }
    else
    {
        // Do nothing
    }
}

