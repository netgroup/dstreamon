

import fields
import string
import random
import re

# SHARED VARIABLES

tokens = fields.Fields("messages.xml")
metrics = dict()
features = dict()
utilities = ["ts","state.code", "state.level", "state.description", "client.id"]
tables = dict()
timeouts = dict()
attacks = dict()

table_clist = set()

__charset = string.letters + string.digits


class PrintUtils(object):
    
    _regex = None

    @classmethod
    def replace(self, body, on_match):
        if not PrintUtils._regex:
            feats_tokens = '|'.join(features)
            field_tokens = '|'.join(tokens.names())
            utilities_token = '|'.join(utilities)

            all_tokens = "{0}|{1}|{2}".format(field_tokens, feats_tokens, utilities_token)

            PrintUtils._regex = "%({0})".format(all_tokens)

        return re.sub(PrintUtils._regex, on_match, body)

# SHARED FUNCTIONS


def __on_token_match(mo):
    
    tkname = mo.group(1)

    return "Tags->Map[{0}]".format( tokens[tkname] )


def on_table_match(mo):

    tname, tkey = mo.groups()

    if mo.groups() in table_clist:        
        return "*tables.{0}->get({1})".format(tname, tkey)
    else:
        return "tables.{0}->get({1})".format(tname, tkey)


def GetRandomString():
    return "".join( random.choice(__charset) for x in xrange(8) )


def ReplaceTokens(body):
    
    __tokens_regex = r"\b({0})\b".format( '|'.join(tokens.names()) )

    return re.sub(__tokens_regex, __on_token_match, body)

# SHARED CLASSES

class Table(object):
    attributes = [ "name", "type", "key_type", "value_type", "nhash", "shash" ]

    elements = [ "entry" ]

    _count = 0

    _decl   = []
    _cons   = []
    _resets = []

    #_get_regex_template = "({0})[(](\w+)[)]"

    _get_regex_template = "({0})\[(\w+)\]"

    _get_regex = ""


    @classmethod
    def regex_get(self):
        if not self._get_regex:
            self._get_regex = self._get_regex_template.format( '|'.join(tables) )

        return self._get_regex

    @classmethod
    def try_parse_get(self, stmt):
        mo = re.match(self.regex_get(), stmt)

        if mo:
            tname, tkey = mo.groups()

            if tkey not in tokens:
                raise Exception('table key not valid')
            
            return (tname, tkey)

        return (None, None)
        

    @classmethod
    def declaration(self):
        return \
"""
struct secondary_tables
{{
    {0}

    secondary_tables()
    {{
        // init tables
        {1}
    }}

}} tables;
""".format( "\n    ".join(Table._decl), "\n    ".join(Table._cons) )


    @classmethod
    def reset_codes(self):
        return "\n".join(self._resets)


    def _declare(self):
        return "IDictionary<{value_type}>* {name};".format(**self.values)

    def _construct(self):
        return "{name} = new {type}<{targs_s}>({args});".format(**self.values)

    def _reset_code(self):
        return \
"""
int table_{name}_reset(const struct pptags* Tags)
{{
    tables.{name}->clear();

    return CUSTOM_ACTION;
}}
""".format(**self.values)
                                                        


    def get_code(self):
        return \
"""
{value_type} {name}(int index, const struct pptags* Tags)
{{
    auto& fk = Tags->Map[ index ];

    auto Ptr = Tags->Tables[0]->get( fk );

    return ( Ptr ? *Ptr : "" );
}}
""".format(**self.values)


    def __init__(self, table_obj):
        
        self.values = dict()

        for Attr in self.attributes:
            self.values[Attr] = table_obj.getAttribute(Attr)

        for Elem in self.elements:
            self.values[Elem] = table_obj.getElementsByTagName(Elem)

        # self.__dict__ = self.values

        self.name = self.values["name"]

        # STORE VALUE-TYPE FOR FUTURE USAGE

        if self.name in tables:
            raise SyntaxError("two tables with same name => {0}".format(self.name))
        else:
            tables[ self.name ] = self.values["value_type"]

        """
        if self.name not in self._tokens:
            self._tokens[ self.name ] = self.values["id"] = self._count
            self._count += 1
        """

        targs = []

        ktype = self.values["key_type"]
        vtype = self.values["value_type"]
        nhash = self.values["nhash"]
        shash = self.values["shash"]

        if ktype: targs.append(ktype)
        if vtype: targs.append(vtype)

        assert 0 < len(targs) <= 2

        self.values["targs_s"] = ",".join(targs)

        self.values["args"] = ""


        if self.values["type"] == "DLeft":
            self.values["args"] = "{0}, {1}".format(nhash, shash)


        Table._decl.append( self._declare() )
        Table._cons.append( self._construct() )
        Table._resets.append( self._reset_code() )

        entries = ""

        for en in self.values["entry"]:
            key = en.getAttribute("key")
            value = en.getAttribute("value")

            entries += """<entry key="{0}" value="{1}" />""".format( key, value )

        self.values["entries"] = entries
 
    def __str__(self):
        return \
"""
<table id="0" value_type="{value_type}" nhash="{nhash}" shash="{shash}">
    {entries}
</table>""".format(**self.values)


if __name__ == "__main__":
    print tokens.names()
