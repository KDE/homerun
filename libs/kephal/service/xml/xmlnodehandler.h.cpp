/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */



namespace Kephal {

    template <class ElementType, typename SimpleType>
    XMLSimpleNodeHandler<ElementType, SimpleType>::XMLSimpleNodeHandler(Getter getter, Setter setter) {
        m_setter = setter;
        m_getter = getter;
    }

    template <class ElementType, typename SimpleType>
    void XMLSimpleNodeHandler<ElementType, SimpleType>::beginSave(XMLType * element) {
        Q_UNUSED(element)
        m_saved = false;
    }

    template <class ElementType, typename SimpleType>
    void XMLSimpleNodeHandler<ElementType, SimpleType>::beginLoad(XMLType * element) {
        Q_UNUSED(element)
    }

    template <class ElementType, typename SimpleType>
    bool XMLSimpleNodeHandler<ElementType, SimpleType>::hasMore(XMLType * element) {
        Q_UNUSED(element)
        return ! m_saved;
    }

    template <class ElementType, typename SimpleType>
    void XMLSimpleNodeHandler<ElementType, SimpleType>::setNode(XMLType * element, QDomNode node) {
        QDomNode child = node.firstChild();
        if (! child.isNull()) {
            ((ElementType *)element->*m_setter)(toValue(child.nodeValue()));
        }
    }

    template <class ElementType, typename SimpleType>
    QDomNode XMLSimpleNodeHandler<ElementType, SimpleType>::node(XMLType * element, QDomDocument doc, QString name) {
        m_saved = true;
        QDomNode node = doc.createElement(name);
        node.appendChild(doc.createTextNode(toString(((ElementType *)element->*m_getter)())));
        return node;
    }

    template <class ElementType, typename SimpleType>
    QString XMLSimpleNodeHandler<ElementType, SimpleType>::str(XMLType * element) {
        return toString(((ElementType *)element->*m_getter)());
    }



    template <class ElementType>
    XMLStringNodeHandler<ElementType>::XMLStringNodeHandler(Getter getter, Setter setter) : XMLSimpleNodeHandler<ElementType, QString>(getter, setter) {
    }

    template <class ElementType>
    QString XMLStringNodeHandler<ElementType>::toValue(QString str) {
        return str;
    }

    template <class ElementType>
    QString XMLStringNodeHandler<ElementType>::toString(QString str) {
        return str;
    }



    template <class ElementType>
    XMLIntNodeHandler<ElementType>::XMLIntNodeHandler(Getter getter, Setter setter) : XMLSimpleNodeHandler<ElementType, int>(getter, setter) {
    }

    template <class ElementType>
    int XMLIntNodeHandler<ElementType>::toValue(QString str) {
        return str.toInt();
    }

    template <class ElementType>
    QString XMLIntNodeHandler<ElementType>::toString(int i) {
        return QString::number(i);
    }



    template <class ElementType>
    XMLUIntNodeHandler<ElementType>::XMLUIntNodeHandler(Getter getter, Setter setter) : XMLSimpleNodeHandler<ElementType, unsigned int>(getter, setter) {
    }

    template <class ElementType>
    unsigned int XMLUIntNodeHandler<ElementType>::toValue(QString str) {
        return str.toUInt();
    }

    template <class ElementType>
    QString XMLUIntNodeHandler<ElementType>::toString(unsigned int i) {
        return QString::number(i);
    }



    template <class ElementType>
    XMLDoubleNodeHandler<ElementType>::XMLDoubleNodeHandler(Getter getter, Setter setter) : XMLSimpleNodeHandler<ElementType, double>(getter, setter) {
    }

    template <class ElementType>
    double XMLDoubleNodeHandler<ElementType>::toValue(QString str) {
        return str.toDouble();
    }

    template <class ElementType>
    QString XMLDoubleNodeHandler<ElementType>::toString(double i) {
        return QString::number(i);
    }



    template <class ElementType>
    XMLBoolNodeHandler<ElementType>::XMLBoolNodeHandler(Getter getter, Setter setter) : XMLSimpleNodeHandler<ElementType, bool>(getter, setter) {
    }

    template <class ElementType>
    bool XMLBoolNodeHandler<ElementType>::toValue(QString str) {
        return str == "true" || str == "t" || str == "on" || str == "1";
    }

    template <class ElementType>
    QString XMLBoolNodeHandler<ElementType>::toString(bool b) {
        return b ? "true" : "false";
    }



    template <class ElementType, class ComplexType>
    XMLComplexNodeHandler<ElementType, ComplexType>::XMLComplexNodeHandler(XMLFactory * factory, Setter setter) {
        m_factory = factory;
        m_setter = setter;
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexNodeHandler<ElementType, ComplexType>::beginSave(XMLType * element) {
        Q_UNUSED(element)
        m_saved = false;
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexNodeHandler<ElementType, ComplexType>::beginLoad(XMLType * element) {
        Q_UNUSED(element)
    }

    template <class ElementType, typename ComplexType>
    bool XMLComplexNodeHandler<ElementType, ComplexType>::hasMore(XMLType * element) {
        Q_UNUSED(element)
        return ! m_saved;
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexNodeHandler<ElementType, ComplexType>::setNode(XMLType * element, QDomNode node) {
        ComplexType * complex = (ComplexType *) m_factory->load(node);
        ((ElementType *)element->*m_setter)(complex);
    }

    template <class ElementType, typename ComplexType>
    QDomNode XMLComplexNodeHandler<ElementType, ComplexType>::node(XMLType * element, QDomDocument doc, QString name) {
        Q_UNUSED(element)
        m_saved = true;
        QDomNode node;
        return node;
    }

    template <class ElementType, typename ComplexType>
    QString XMLComplexNodeHandler<ElementType, ComplexType>::str(XMLType * element) {
        Q_UNUSED(element)
        return "";
    }



    template <class ElementType, class ComplexType>
    XMLComplexListNodeHandler<ElementType, ComplexType>::XMLComplexListNodeHandler(XMLFactory * factory, ListGetter listGetter) {
        m_factory = factory;
        m_listGetter = listGetter;
    }

    template <class ElementType, typename ComplexType>
    bool XMLComplexListNodeHandler<ElementType, ComplexType>::hasMore(XMLType * element) {
        return m_pos < ((ElementType *)element->*m_listGetter)().size();
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexListNodeHandler<ElementType, ComplexType>::setNode(XMLType * element, QDomNode node) {
        ComplexType * complex = (ComplexType *) m_factory->load(node);
        ((ElementType *)element->*m_listGetter)().append(complex);
    }

    template <class ElementType, typename ComplexType>
    QDomNode XMLComplexListNodeHandler<ElementType, ComplexType>::node(XMLType * element, QDomDocument doc, QString name) {
        ComplexType * complex = ((ElementType *)element->*m_listGetter)().at(m_pos);
        ++m_pos;
        return m_factory->save(complex, doc, name);
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexListNodeHandler<ElementType, ComplexType>::beginSave(XMLType * element) {
        Q_UNUSED(element)
        m_pos = 0;
    }

    template <class ElementType, typename ComplexType>
    void XMLComplexListNodeHandler<ElementType, ComplexType>::beginLoad(XMLType * element) {
        ((ElementType *)element->*m_listGetter)().clear();
    }

    template <class ElementType, typename ComplexType>
    QString XMLComplexListNodeHandler<ElementType, ComplexType>::str(XMLType * element) {
        Q_UNUSED(element)
        return "";
    }

}

