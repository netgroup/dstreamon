
import shared
import re

class Actions(object):

    _template = "action_function {0}[] = {{ {1}, NULL }};\n"

    _raw_regex = "({0})[(]({1})[)][ ]*([<>=!*+/][=]?)?[ ]*(.*)"

    _regex = ""


    @classmethod
    def regex(self):
        if not Actions._regex:
            Actions._regex = self._raw_regex.format('|'.join(shared.tables), \
                '|'.join(shared.tokens.names()))
 
        return Actions._regex


    @classmethod
    def generate(self, names):

        if not names:
            return "NULL", ""

        randname = "ActionList_{0}".format(shared.GetRandomString())

        return randname, self._template.format(randname, ", ".join(names))


    @classmethod
    def _sub_print(self, match_object):

        key = match_object.group(1)

        if key in shared.tokens:            
            key_id = shared.tokens[key]
            sub =  "\" << Tags->Map[{0}].GetString() << \"".format(key_id)
        elif key in shared.features:
            key_id = shared.features[key]
            sub = "\" << std::to_string(Tags->Features[{0}]) << \"".format(key_id)
        elif key in shared.utilities:
            if key == 'ts':
                sub = \
    '''\" << timestamp() << \"'''
            elif key == 'state.code':
                sub = '''\" << getStateCode(Tags) << \"'''
            elif key == 'state.level':
                sub = '''\" << getStateLevel(Tags) << \"'''
            elif key == 'state.description':
                sub = '''\" << getStateDescription(Tags) << \"'''
            elif key == 'client.id':
                sub = '''\" << getClientId() << \"'''
        return sub


    @classmethod
    def parse_execute(self, body):

        name = "Execute_{0}".format( shared.GetRandomString() )

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    Buffer<256> command;

    command << "{1}" << 0U;

    system( (const char*) command.GetRawData());

    return CUSTOM_ACTION;
}}
""".format(name, shared.PrintUtils.replace(body, Actions._sub_print))

        return name, res

    @classmethod
    def parse_print(self, body):
        """
        feats_tokens = '|'.join(shared.features)
        field_tokens = '|'.join(shared.tokens.names())

        all_tokens = "{0}|{1}".format(field_tokens, feats_tokens)

        prex = "%({0})".format(all_tokens)
        """

        name = "Print_{0}".format( shared.GetRandomString() )

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    std::cout << "{1}\\n";

    return CUSTOM_ACTION;
}}

""".format(name, shared.PrintUtils.replace(body, Actions._sub_print))

        
        # res = "std::cout << \""

        #res += re.sub(prex, Actions._sub_print, body)

        return name, res

    @classmethod
    def parse_publish(self, body):

        body = body.split(',')
        if len(body) != 2:
            raise Exception('Publish got ' + str(len(body)) + ' params, expected 2: ' + str(body))
        channel = body[0]
        expr = body[1]

        name = "Publish_{0}".format( shared.GetRandomString() )

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    Tags->zmq_publish["{1}"] = "{2}";

    return EXPORT_PACKET;
}}

""".format(name, channel, expr)

        if expr != "raw":
            res = \
"""
int {0}(const struct pptags* Tags)
{{
    std::stringstream o;
    o << "{2}";
    Tags->zmq_publish["{1}"] = o.str();

    return EXPORT_PACKET;
}}

""".format(name, channel, shared.PrintUtils.replace(expr, Actions._sub_print))

        return name, res

    @classmethod
    def parse_export(self, body):

        args = ['raw', '']

        if body:
            args = body.split(',')
            if len(args) != 2:
                raise Exception('Export got ' + str(len(args)) + ' params, expected 2: ' + str(args))

            args[0] = args[0].strip()

            if args[0] == 'raw':
               args[1] = '' 

            args[1] = args[1].strip()

        name = "Export_{0}".format( shared.GetRandomString() )

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    Tags->export_channel = "{1}";
    Tags->export_message = "{2}";

    return EXPORT_PACKET;
}}

""".format(name, args[0], args[1])

        
        # res = "std::cout << \""

        #res += re.sub(prex, Actions._sub_print, body)

        return name, res

    @classmethod
    def parse_plot(self, body):
        
        feat_tokens = '|'.join(shared.features)
        field_tokens = '|'.join(shared.tokens.names())

        prex = "(\w+)[ ]({0})[ ](.+)".format(field_tokens)

        mo = re.search(prex, body)

        if mo:
            stream, key, fnames = mo.groups()
        else:
            return (None, None)

        name = "Plot_{0}".format( shared.GetRandomString() )

        # find key_id

        key_id = shared.tokens[key]

        # now parsing feature names

        elements = fnames.split(' ')

        elem_template_code = '{{\\"name\\":\\"{0}\\",\\"value\\":%f}}'

        elem_templates = []
        concrete_feat = []

        for e in elements:
            elem_templates.append(elem_template_code.format(e))
            concrete_feat.append("Tags->Features[{0}]".format(shared.features[e]))

        elem_template_s = "[{0}]".format(','.join(elem_templates))

        features_tags = ",".join(concrete_feat) 

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    static int piped = -1;

    static const char* _json_template = "{{\\"stream\\" : \\"{1}\\", \\"key\\" : \\"%s\\", \\"elements\\" : {3} }}\\n";


    if (piped == -1)
    {{
        mode_t mode = S_IRWXU|S_IRWXG|S_IRWXO;

        //mkfifo("{1}", mode);

	piped = open("{1}", O_WRONLY);
    }}

    if (piped != -1)
    {{
        char elems[1024];

        // fill elem template

        int written = snprintf(elems, 1024, _json_template, Tags->Map[{2}].GetString().c_str(), {4});

        // send _json_data through pipe p
        write(piped, elems, written);
    }}
    else
    {{
        throw std::runtime_error("[featurelib] ERROR: Pipe not ready!");
    }}

    return CUSTOM_ACTION;
}}
""".format(name, stream, key_id, elem_template_s, features_tags)

        return name, res

    @classmethod
    def parse_metric_reset(self, body):
    
        metric_tokens = '|'.join(shared.metrics)
        field_tokens  = '|'.join(shared.tokens.names())

        prex = "({0})[\.]reset[(]({1})?[)]".format(metric_tokens, field_tokens)

        mo = re.search(prex, body)

        if mo:
            metric_name, reset_key = mo.groups()
        else:
            return (None, None)

        name = "ResetMetric_{0}".format( shared.GetRandomString() )        

        metric_id = shared.metrics[metric_name]
        
        if reset_key:

            reset_key = "Tags->Map[{0}]".format(shared.tokens[reset_key])

            reset_body = "Tags->ResetMetrics[{0}].VMResetKey = {1};".format(metric_id, reset_key)
        else:
            reset_body = ""

        res = \
"""
int {0}(const struct pptags* Tags)
{{
    Tags->ResetMetrics[{1}].VDReset = 1;
    Tags->ResetMetrics[{1}].VMReset = 1;

    {2}

    return CUSTOM_ACTION;
}}
""".format(name, metric_id, reset_body)

        return name, res


    @classmethod
    def initMap(self):
        Actions._Map = \
        { "print"  : Actions.parse_print,
          "plot"   : Actions.parse_plot,
          "exec" : Actions.parse_execute,
          "Export" : Actions.parse_export,
          "Publish" : Actions.parse_publish
        }
    
    @classmethod
    def parse(self, acts):

        Actions.initMap()

        action_regex = Actions.regex()

        table_regex = "({0})([\.]reset)".format('|'.join(shared.tables))
        
        output = ""

        actionlist = acts.split(';')

        actionnames = []
        actionbodies = []

        if not actionlist[0]:
            return (actionnames, actionbodies)


        for action in actionlist:

            mo = re.match(action_regex, action)

            if mo:
                tname, tkey, top, tvalue = mo.groups()

                if not top or not tvalue: break

                vptr = ""

                value = tvalue

                trandname = "{0}_action_{1}".format(tname, shared.GetRandomString() )

                actionnames.append(trandname)

                cname, ckey = shared.Table.try_parse_get(tvalue)

                if cname:
                    vptr = "*"
                    value = "TGET(tables.{0}, Tags->Map[{1}])".format( cname, shared.tokens[ckey] )
                elif tvalue in shared.features:
                    value = "Tags->Features[{0}]".format( shared.features[tvalue] )
                    # also add to features to calculate!

                tcode = \
"""
int {0}(const struct pptags* Tags)
{{
    auto cur = TGET(tables.{4}, Tags->Map[{1}]);

    auto val = {3};

    if (!cur || !val)
    {{        
        cur = tables.{4}->add(Tags->Map[{1}], std::remove_pointer<decltype(val)>::type());
    }}

    if (val)
        *cur {2} {5}val;

    return CUSTOM_ACTION;
}}

""".format(trandname, shared.tokens[tkey], top, value, tname, vptr)

                actionbodies.append(tcode)

            else:
                # is a standard action ?
				# here we try to match a standard function body, then it will call
				# the correct handler (if exists) and pass it the args

                pact = re.match("(\w+)[(](.*)[)]", action)

                if pact:

                    actname, actbody = pact.groups()

                    handler = Actions._Map[actname]

                    name, res = handler(actbody)

                    actionnames.append(name)
                    actionbodies.append(res)
                    # print res
                else:
                    name, res = Actions.parse_metric_reset(action)

                    if name:
                        # is a metric reset action ?

                        actionnames.append(name)
                        actionbodies.append(res)

                    else:
                        #substitution try to find
                        action = re.sub(table_regex, 'table_\g<1>_reset', action)
                        actionnames.append(action)
        """
        if actionlist[0]:
            output = self._template.format( shared.GetRandomString(), \
                                               ", ".join(actionnames) )
        """

        return (actionnames, actionbodies)


if __name__ == "__main__":

    #shared.tables['tt'] = 1

    shared.metrics['pippa'] = 9
    shared.features['pippo'] = 1
    shared.features['pluto'] = 2
    shared.features['minni'] = 3

    #test_action = "plot(out ip_src pippo pluto)"

    #test_action = "exec(iptables %ip_src)"

    test_action = "Export()"

    actionlist, actionbodies = Actions.parse(test_action)

    #print actionlist
    print actionbodies[0]

    #name, res = Actions.parse_print("src: %ip_src, feat: %pippo")

    #name, res = Actions.parse_metric_reset("pippa.reset()")

    #name, res = Actions.parse_plot("out ip_src pippo pluto minni")

    #print name
    #print res
