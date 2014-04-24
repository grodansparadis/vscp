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

#ifndef EXCEL_H
#define EXCEL_H

#include <QtGui>
#include <QStandardItemModel>
#include <QtXml>

#define FAIL_TO_OPEN_FILE      0
#define SUCCESS_TO_OPEN_FILE   1

class xmlLib : public QObject
{

public:
    xmlLib();
    ~xmlLib();

    /*------------------- xml file functions -------------------*/
    // open xml file
    int xmlOpenFile( QString &xmlFileName, QFile &xmlFile, QTextStream &xmlStream);
    // close xml file
    int xmlOpenFileTruncate( QString &xmlFileName, QFile &xmlFile, QTextStream &xmlStream);
    //Load the XML file
    void xmlLoadFile(QDomDocument &xmlDomDocument, QFile &xmlFile);
    //close file
    void xmlCloseFile(QTextStream &xmlStream, QDomDocument &xmlDomDocument, QFile &xmlFile);
    //close file no flush
    void xmlCloseFileNoFlush(QFile &xmlFile);

    // xml node functions
    //Add the root item
    void xmlAddRoot(QDomElement &xmlRoot, QDomDocument &xmlDomDocument);
    //Get the root item
    void xmlGetRoot(QDomElement &xmlRoot, QDomDocument &xmlDomDocument);
    // Create the node
    void xmlCreateNode(QDomDocument &xmlDomDocument, QDomElement &NodeElement, QString Name);
    // return number of direct root childs
    int xmlRootDirectChildNode(QDomDocument &xmlDomDocument);
    // find number of elements attached to the node
    int xmlTotalChildNode(QDomNode &NodeElement);
    // Reading particular child node from the parent
    void xmlGetNodeByNumber(QDomNode Parent, int nodeNumber, QDomNode &temp);
    // Create the text node
    void xmlAddTextNode(QDomDocument &xmlDomDocument, QDomElement &NodeElement, QString textContent);
    //Append node to the parent
    void xmlAppendNode(QDomElement &xmlParent, QDomElement &xmlChild);
    // Replace old node with new one
    void xmlUpdateNode(QDomElement &xmlRoot, QDomNode &newNode,QDomNode &oldNode);
    // Remove child from xml Root -- i.e -- remove test case
    void xmlRootDeleteNode(QDomElement &xmlRoot, QDomNode &nodeToRemove);
    // Set the value of the xml node
    void xmlWriteNodeValue(QDomElement &NodeElement, QString NodeValueTxt);
    // Read the value of the xml node
    void xmlReadNodeValue(QDomNode &NodeToReadValue, QString NodeValueTxt);
    // Reading particular child node from the parent
    void xmlRootRemoveNodeByNumber(QDomElement &xmlRoot, int nodeNumber);
protected:


private:

};

#endif
