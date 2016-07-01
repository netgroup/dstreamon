import xml.dom.minidom

class CompositionParser:
    """\brief Utility class for parsing a blockmon composition
    """

    def __init__(self, comp=None):
        """\brief Initializes class
        \param comp (\c string) The composition
        """
        self.__comp = comp

    def set_comp(self, comp):
        self.__comp = comp

    #<composition id="arch_input" app_id="boh">
    def parse_comp_id(self):
        """\brief Returns the composition's id
        \return (\c string) The composition's id
        """
        dom = self.__get_DOM(self.__comp, False)
        if (dom == None):
            print "bmparser::parse_comp_id: error while getting DOM object"
            return None
        comp_xml = dom.getElementsByTagName("composition")[0]
        return self.__get_label("id", comp_xml)

    def __get_text(self, nodelist):
        """\brief Concatenates text data from a list of xml nodes
        \param nodelist (\c minidom.Node[]) The nodes
        \return         (\c string)         The concatenated text
        """
        text = ""
        for node in nodelist:
            if node.nodeType == node.TEXT_NODE:
                text += node.data
        return text

    def __get_DOM(self, desc, file=True):
        """\brief Turns an xml file into a DOM object. If the file parameter is set to
                  true, desc should be the path to the xml file to read. Otherwise, desc
                  is a string containing xml to turn into a DOM object.
        \param desc  (\c string)  Path to an xml file or a string containing xml
        \param file  (\c bool)    Whether desc is a file or an xml string (default is true)
        \return      (\c xml.dom.minidom.Document) The DOM object
        """
        dom = None
        try:
            if file:
                dom = xml.dom.minidom.parse(desc)
            else:
                dom = xml.dom.minidom.parseString(desc)
        except Exception, e:
            print "Error getting dom " + str(e)
            return None
        return dom

    def __get_label(self, key, xml_object):
        """\brief Given an xml object and a key, returns the value matching that key
                  (a string) or None if nothing matches the key.
        \param key        (\c string)        The key to search for
        \param xml_object (\c minidom.Node)  The xml object to search for the key in
        \return           (\c string)        The value found or None if no value was found for the given key
        """
        if xml_object.attributes.has_key(key):
            return xml_object.attributes[key].value
        else:
            return None
