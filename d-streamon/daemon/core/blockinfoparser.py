import os
import sys
import ConfigParser
import xml.dom.minidom

class BlockInfoParser:

    """\brief Crawls blockmon's source tree for block files and creates a python
              file containing a dictionary of block types to core.block.BlockInfo
              objects. This script takes the root of the source tree as input and
              only crawls the "blocks" directory, looking in the hpp or cpp files
              for comments containing XML inside <blockinfo> tags. The dictionary
              generated can be imported from written to core.blockinfo.py.block_infos.
    """
    def __init__(self):
        pass

    def generate_blockinfo(self, blocks_path, base_path):
        """\brief Generates block information from source code comments
        \param blocks_path (\c string) Blockmon's path to the blocks directory
        \param base_path   (\c string) Blockmon's base path
        """
        # create output file and write beginning of it
        info_str = "from block import BlockInfo, GateInfo, VariableInfo, IntegerRange\n\nblock_infos = {"

        # process files
        for root, subFolders, files in os.walk(blocks_path):
            for file in files:
                if file.endswith(".hpp") or \
                   file.endswith(".cpp"):                    
                    src_file = os.path.join(root,file)
                    print "processing " + str(src_file)
                    f = open(src_file, "r")
                    lines = f.read().splitlines()
                    f.close()
                    in_block_info = False
                    info = []

                    for line in lines:
                        if line.strip().startswith("*"):                            
                            if line.find("<blockinfo") != -1:
                                print "found documentation, processing"
                                in_block_info = True
                            if line.find("</blockinfo") != -1:
                                info.append(line.replace("*", ""))
                                in_block_info = False
                        
                        if in_block_info:
                            info.append(line.replace("*", ""))
                    
                    if len(info) > 0:
                        info_str += self.__append_blockinfo(info) + ', '
        
        info_str = info_str[:len(info_str) - 2]

        # append end of file and write it out
        info_str += "}\n"
        output_file = base_path + "/daemon/core/blockinfo.py"
        f = open(output_file, "w")
        f.write(info_str)
        f.close()
        print "wrote info to " + output_file

    def __append_blockinfo(self, lines):
        """\brief Returns a string containing a block's info
        \param lines (\c list[string]) The lines to parse
        \return      (\c string)       The block's info
        """
        (trimmed, params_example) = self.__extract_section("paramsexample", lines)
        (trimmed, params_schema) = self.__extract_section("paramsschema", trimmed)
        
        xml_str = ""
        for line in trimmed:
            xml_str += line + "\n"
        
        dom = self.__get_DOM(xml_str, False)
        if (dom == None):
            print "blockinforparser::append_blockinfo: error while getting DOM object"
            return None

        blockinfo_xml = dom.getElementsByTagName('blockinfo')[0]
        block_type = self.__get_label("type", blockinfo_xml)
        string = '"' + block_type + '": BlockInfo("' + block_type + '", '

        scheduling_type = self.__get_label("scheduling_type", blockinfo_xml)
        string += '"' + scheduling_type + '", '

        string += self.__append_gates_info(dom.getElementsByTagName("gate"))

        string += ', "' + params_schema + '", "' + params_example + '", '

        string += self.__append_variables_info(dom.getElementsByTagName("variable"), block_type)

        human_desc = dom.getElementsByTagName('humandesc')[0].firstChild.nodeValue
        string += ', "' + human_desc.replace("\n", "").replace('"', "'").strip() + '"'

        short_desc = dom.getElementsByTagName('shortdesc')[0].firstChild.nodeValue
        string += ', "' + short_desc.replace("\n", "").replace('"', '\"').strip() + '", '

        thread_exclusive = self.__get_label("thread_exclusive", blockinfo_xml)
        if thread_exclusive.lower() == "true":
            thread_exclusive = True
        else:
            thread_exclusive = False
        string += str(thread_exclusive) + ")"

        return string
 

    def __append_gates_info(self, gates_xml):
        """\brief Given parsed XML with information about gates, returns a
                  string with that information
        \param gates_xml (\c Node)   An XML node with the gate information
        \return          (\c string) The gate information
        """
        string = "["
        for gate_xml in gates_xml:
            type = self.__get_label("type", gate_xml)
            name = self.__get_label("name", gate_xml)
            msg_type = self.__get_label("msg_type", gate_xml)
            multiplicity_start = self.__get_label("m_start", gate_xml)
            multiplicity_end = self.__get_label("m_end", gate_xml)
            string += 'GateInfo("' + type + '", ' + \
                               '"' + name + '", ' + \
                               '"' + msg_type + '", ' + \
                               'IntegerRange(' + multiplicity_start + ', ' + \
                               multiplicity_end + ')), '
        string = string[:len(string) - 2] + "]"
        return string

    def __append_variables_info(self, variables_xml, block_type):
        """\brief Given parsed XML with information about block variables,
                  returns a string with that information
        \param variables_xml (\c Node)   An XML node with the variables information
        \param block_type    (\c string) The block type
        \return              (\c string) The variables information
        """        
        string = "["
        for variable_xml in variables_xml:
            name = self.__get_label("name", variable_xml)
            human_desc = self.__get_label("human_desc", variable_xml)
            access = self.__get_label("access", variable_xml)
            string += 'VariableInfo("' + block_type + '", ' + \
                                   '"' + name + '", ' + \
                                   '"' + human_desc + '", ' + \
                                   '"' + access + '"), '
        if len(variables_xml) > 0:
            string = string[:len(string) - 2]

        return string + "]"

    def __extract_section(self, section_name, lines):
        """\brief Extracts a section enclosed in XML < /> angled brackets.
                  The function returns a tuple whose first object is a list
                  of strings represented the given lines with the section
                  extracted; the second object is a string containing the section
        \param section_name (\c string)       The section's name
        \param lines        (\c list[string]) The lines to parse
        \return             (\c tuple(list[string],string)) The extracted section
        """
        in_section = False
        section = ""
        trimmed = []
        for line in lines:
            if line.find("<" + section_name) != -1:
                in_section = True
            if line.find("</" + section_name) != -1:
                in_section = False
                section += line + '\\n'
            
            if in_section:
                section += line + '\\n'
            elif line.find("</" + section_name) == -1:
                trimmed.append(line)
        section = section.replace('"', "'")
        return (trimmed, section)


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
        


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "usage: blockinfoparser.py [config]"
        os._exit(1)

config = sys.argv[1]
cp = ConfigParser.ConfigParser()
cp.read(config)
blocks_path = cp.get('BLOCKS', 'blocks_path')
base_path = cp.get('DEFAULT', 'bm_basepath')
parser = BlockInfoParser()
parser.generate_blockinfo(blocks_path, base_path)


