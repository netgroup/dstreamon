
# EVENT.PY
# 
# THIS MODULE IMPLEMENTS SELECTOR GRAMMAR (showed below)
#
# selector   := stmt*
# stmt       := proto.field [op] value [ or | and ]
# proto      := ip | udp | tcp | dns
# field      := src | dst | src-port | dst-port | domain | ipaddress
# op         := comparison | function
# comparison := < | > | == | >= | <= | !=

# function   := IN | RANGE
# IN         := in(dictionary)
# RANGE      := range(value, value)
# dictionary := (table names)
# value      := [a-z]*[A-Z]*[0-9]*
#


import re
import shared

class Event(object):
    _attributes = [ "type", "primary-key", "source", "class", "selector", "attack_type" ] # "source", "class"

    _count = 0
    _last  = 0

    _operators = {'<', '<=', '==', '!=', '>=', '>'}

    _proto_v = \
    {
        'tcp' : '6',
        'udp' : '17'
    }

    _flags_v = \
    {
        'tcp-syn'         : '0x02',
        'tcp-ack'         : '0x10',
        'dns-noerror'     : '0x00',
        'dns-formaterr'   : '0x01',
        'dns-failed'      : '0x02',
        'dns-nxdomain'    : '0x03',
        'dns-nimplemented': '0x04'   
    }

    _http_method_v = \
    {
        'http-get'        : '0x20544547U',
        'http-head'       : '0x44414548U',
        'http-post'       : '0x54534F50U'
    }

    _symbols = { "proto" : _proto_v, "flags" : _flags_v, "http_method" : _http_method_v }

#                      field     oper                   value     
    _selector_regex = "(\S+)[ ]*([<>!=][=]?)?[ ]*(\S+)[ ]*(or|and)?"

    @classmethod
    def last(self):
        return self._last

    @classmethod
    def parse_operators(self, filename):
        regex = "^\S+[ ]+(operator)?(\S+)[ ]*[(]"

        op_set = set()

        f = open(filename)

        for row in f:
            mo = re.search(regex, row)

            if mo:
                op_set.add( mo.group(2) )

        f.close()

        return "|".join(op_set)

        

    @classmethod
    def parse_selector(self, selector):

        output = []

        whole_ops = self.parse_operators("operators.hpp")

        statements = re.findall(self._selector_regex, selector)

        if not statements:
            output.append("true")
        
        for stmt in statements:
            field, op, value, more = stmt

            #print stmt

            # select the right replacement table due to field name
            replacement_table = self._symbols.get(field, dict())
                        
            field_s = "Tags->Map[{0}]".format( shared.tokens[field] )
            # op parsing

            if not op:
                op = "operator=="
            elif op in self._operators:
                op = "operator{0}".format(op)
            elif op not in whole_ops:
                raise KeyError("\"{0}\" is not a valid operator".format(op))

            # value replacement

            if value in shared.tables:
                value_s = "tables.{0}".format(value)
            elif replacement_table:        #value in replacement_table:
                vlist = value.split('|')

                try:
                    value_s = "|".join( replacement_table[value] for value in vlist )
                except KeyError:
                    print "Error: {0} not a valid value for {1}".format(value, field)
            else:
                value_s = value
            
            op_s = "{0}({1}, {2}) {3} ".format(op, field_s, value_s, more)

            output.append(op_s)

        return "[](const pptags* Tags) {{ return {0}; }}".format("".join(output))

    @classmethod
    def parse(self, evt):

        self._last = self._count

        self._count += 1

        values = dict()

        for Attr in self._attributes:
            values[Attr] = evt.getAttribute(Attr)

        # output = ""

        if values["type"] == "timeout":                       
            selector = "[](const pptags* Tags) {{ return false; }}"
        else:
            selector = self.parse_selector( values["selector"] )

        event_name = "Event_{0}".format(self._last)

        table_id = -1

        pk = values["primary-key"]

        if pk not in shared.tokens:
            shared.tokens.add_composite(pk)

        key_id = shared.tokens[pk]

        atype = shared.attacks.get( values["attack_type"], 1 )

        output = "{{ {0}, {1}, {{ {2}, {3} }}, {4} }}".format( selector, event_name, table_id, key_id, atype )


        return output



if __name__ == "__main__":

    import sys

    shared.tables["HT1"] = "uint"

    if len(sys.argv) != 2:
        print "Syntax: event.py \"field op value [or|and]\" "
        quit()
    
    #s = "proto == udp and ip_src in HT1 and dst_port < 32"

    l = Event.parse_selector(sys.argv[1])

    print l

