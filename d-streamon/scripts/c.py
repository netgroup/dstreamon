
import shared
import re
import classes
from actions import *

from collections import OrderedDict


class Function(object):
    
    def __str__(self):
        return \
"""
extern "C" {2} {0}( double* Input )
{{
    return {1};
}}

""".format(self.name, self.retvalue, self.rettype)


class Timeout(object):

    attributes = ["class", "key", "value", "primarykey"]

    count = 0

    @classmethod
    def fill_from_events(self, events):

        for index, ev in enumerate(events):

            if ev.getAttribute("type") == "timeout":
                cl = ev.getAttribute("class")

                print "=> class:", cl

                if cl in shared.timeouts:
                    raise Exception("Found two timeout events with the same class!")
                else:
                    shared.timeouts[cl] = index
                
            

    @classmethod
    def _parse_one(self, timeout_obj):

        values = dict()

        for Attr in self.attributes:
            values[Attr] = timeout_obj.getAttribute(Attr)

        if not values['class'] or not values['key']:
            raise Exception("timeout_set: missing class or key attributes")

        # perform substitution if needed

        values['evid'] = shared.timeouts[values['class']]

        # key -> ( plain string or pointer to a buffer )

        key = values['key']

        if key in shared.tokens:
            values['key_header'] = ""
            values['key'] = "Tags->Map[{0}]".format( shared.tokens[key] )
        else:
            values['key_header'] = "static HashedReadBuffer key(\"{0}\");".format(self.values['key'])
            values['key'] = "key"


        # value -> is ok, simple number (eventually to scale)

        value = values['value']

        if not value:
            value = 0

        # primarykey -> (buffer field of packet)

        primarykey = values['primarykey']

        if primarykey and primarykey in shared.tokens:
            values['pkey'] = "&Tags->Map[{0}]".format( shared.tokens[primarykey] )
        else:
            values['pkey'] = "NULL" 

        values['funcname'] = funcname = "Timeout_Set_{0}".format(Timeout.count)

        Timeout.count += 1

        code = \
"""
inline int {funcname}(const struct pptags* Tags)
{{
    {key_header}

    auto tman = Tags->TManager;

    auto value = {value};

    tman->Set( {key},
               value,
               (void*) Tags,
               {evid},
               {pkey} );

    if (!value) return TIMEOUT_REMOVED;

    return TIMEOUT_SETTED;
}}
""".format(**values)


        return (funcname, code)


    @classmethod
    def parse_all(self, timeouts):
    
        names = []
        bodies = []

        for ts in timeouts:
            name, body = self._parse_one(ts)

            names.append(name)
            bodies.append(body)

        return names, bodies
        

class Feature(Function):

    attributes = [ "name", "body" ]

    count = 0

    Table = []

    def __init__(self, feature_obj, metric_table):

        self.values = self.__dict__

        for Attr in self.attributes:
            self.values[Attr] = feature_obj.getAttribute(Attr)
        
        Feature.Table.append( (self.name, Feature.count) )

        Feature.count += 1

        # make sure that metric_table is ordered

        metric_table.sort(key=lambda x: len(x[0]), reverse=True)

        # perform substitutions

        self.rettype = "double"

        for Arg, Index in metric_table:
            self.body = self.body.replace(Arg, 'Input[{0}]'.format(Index))

        self.retvalue = self.body

    @classmethod
    def reset(self):
        Feature.count = 0
        Feature.Table = []


class Condition(object):

    attributes = [ "expression", "next_state", "action" ]

    elements = [ "timeout_set" ]

    List = []

    count = 0

    FeatureSet = [] # OrderedDict()

    _regex = "(.+?(?: [<>][=]? | [!=][=]).+?)($|or|and)"

    _tab_regex = "({0})[(](\w+)[)]"

    tab_regex = None


    @classmethod
    def get_tab_regex(self):

        if not Condition.tab_regex:
            Condition.tab_regex = Condition._tab_regex.format('|'.join(shared.tables))

        return Condition.tab_regex



    @classmethod
    def get_condition_code(self, postfix):
        return \
"""
condition_tuple ConditionList_{0}[] =
{{
       {1}
       {{ NULL, NULL, -1 }}
}};

""".format(postfix, "\n\t".join(Condition.List))


    @classmethod
    def get_features_code(self, postfix):
        if not self.FeatureSet:
            return ""
        
        return \
"""
feature FeatureList_{0}[] = 
{{
   {1},
   {{ -1, NULL }}
}};

""".format(postfix, ",\n    ".join(self.FeatureSet))

    @classmethod
    def reset(self):
        Condition.List = []
        Condition.FeatureSet = [] #.clear()
        Condition.count = 0
        # TableUpdate.reset()
        # Timeout.reset()


    @classmethod
    def get_var_id(self, table, ttype):

        actual_count = len(table)

        if ttype not in table:
            table[ttype] = actual_count

        return table[ttype]
        


    def __init__(self, condition_obj, feature_table, state_table):

        tag_regex = "Tag\((\w+)\)"

        self.values = self.__dict__

        for Attr in self.attributes:
            self.values[Attr] = condition_obj.getAttribute(Attr)

        for Elem in self.elements:
            self.values[Elem] = condition_obj.getElementsByTagName(Elem)

        # fix input objects as needed

        feature_table.sort(key=lambda x: len(x[0]), reverse=True)

                
        # perform substitutions/insertions

        body = self.expression

        for Arg, Index in feature_table:

            feature_present = False

            if Arg in body:
                body = body.replace(Arg, 'Input[{0}]'.format(Index))
                feature_present = True

            if Arg in self.action:
                feature_present = True

            if feature_present:
                Condition.FeatureSet.append("{{ {0}, {1} }}".format(Index, Arg))


        # second pass to find tokens (due to table gets)
        
        declaration_list = []

        tregex = Condition.get_tab_regex()

        # reset list of tables name, key pairs
        shared.table_clist.clear()

        for cond in re.finditer(Condition._regex, body):

            overall, endl = cond.groups()

            protectors = []

            for tabs in re.finditer(tregex, overall): 

                tname, tkey = tabs.groups()

                shared.table_clist.add( tabs.groups() )

                #print "TABLE FOUND => ", tname
                #print "TABLE KEY   => ", tkey

                protector = "tables.{0}->get({1}) and ".format(tname, tkey)

                protectors.append(protector)
    
            body_new = "({0} {1})".format( "".join(protectors), overall)

            body = body.replace(overall, body_new)

        
        body = re.sub(tregex, shared.on_table_match, body)

        body = shared.ReplaceTokens(body)

        condition = "[](double* Input, const struct pptags* Tags) {{ return {0}; }}".format(body)
        
        """
        var_id = 0

        action_regex = Actions.regex()

        for x in re.finditer(action_regex, body):
            tname, tkey, top, tvalue = x.groups()              

            table_dict = dict()

            table_dict['name'] = tname
            table_dict['fid'] = shared.tokens[tkey]
            table_dict['varid'] = var_id
            if top:
                table_dict['op'] = top
            else:
                table_dict['op'] = ""

            var_id += 1 

            ttype = shared.tables[tname]
           
            declaration_list.append("DECL({0}, {1});".format( ttype, table_dict['varid'] ))

            table_template = "(TCHECK(tables.{name}, Tags->Map[{fid}], {varid}) {child_check} && TVALUE({varid}) {op} {child_value})"

            cname, ckey = shared.Table.try_parse_get(tvalue)

            if cname:

                child = dict()

                child['name'] = cname
                child['varid'] = var_id
                child['fid'] = shared.tokens[ckey]

                ctype = shared.tables[cname]

                var_id += 1

                declaration_list.append("DECL({0}, {1});".format(ctype, child['varid']))
                
                table_dict['child_check'] = "&& TCHECK(tables.{name}, Tags->Map[{fid}], {varid})".format(**child)

                table_dict['child_value'] = "TVALUE({varid})".format(**child)
            else:
                table_dict['child_check'] = ""
                table_dict['child_value'] = tvalue

                
            table_statement = table_template.format(**table_dict)
            # table_template = "TCONDGET(tables.\g<1>, Tags->Map[{0}], {1}) \g<3> \g<4>".format(field_id, var_id)

            # print "BODY BEFORE => ", body
           
            body = body.replace(x.group(0), table_statement) 

            # print "BODY AFTER => ", body


        condition = "[](double* Input, const struct pptags* Tags) {{ {0} return {1}; }}".format("".join(declaration_list), body)
        """

        # HANDLING ACTIONS

        timeoutnames, timeoutbodies = Timeout.parse_all( self.timeout_set )

        actionnames, actionbodies = Actions.parse(self.action)

        # MERGE LISTS
        overall_names  = timeoutnames+actionnames
        overall_bodies = timeoutbodies+actionbodies

        setname, setdecl = Actions.generate(overall_names)

        next_state, timeout, _not_used = state_table[ self.next_state ]

        self.tuple = "{{ {0}, {1}, {2} }},".format(condition, setname, next_state)

        Condition.List.append(self.tuple)

        self._actions = overall_bodies
        self._actions += setdecl


    def actions(self):
        return self._actions


    def __str__(self):
        return self.tuple

