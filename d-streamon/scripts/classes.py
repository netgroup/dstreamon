
import shared
import templates
import string
import random
import re

from xml.dom import minidom




class Metric(object):

    attributes = [ "name",  "source", "hw_id", "config" ]

    elements = [ "variation_detector", "variation_monitor" ]
    
    values = dict()

    count = 0

    Table = []

    D = dict()

    def __init__(self, metric_obj):

        self.values = dict()

        #self.__dict__ = self.values

        for Attr in self.attributes:
            self.values[Attr] = metric_obj.getAttribute(Attr)

        for Elem in self.elements:
            self.values[Elem] = metric_obj.getElementsByTagName(Elem)

     
        # perform substitutions

        self.source = self.values.get("source", "")
        self.hw_id = self.values.get("hw_id", "")

        if self.source != 'hw':

            self.values['config'] = templates.ConfigHard.get(self.values['config'], "Metric_Block")

            # add other values

            self.variation_detector = self.values['variation_detector'][0]
            self.variation_monitor = self.values['variation_monitor'][0]

            self.values["vd_status"] = self.variation_detector.getAttribute("status")
            self.values["slave"] = self.variation_detector.getAttribute("slave")
            self.values["reset_after"] = self.variation_detector.getAttribute("reset_after")

            self.values["vm_status"] = self.variation_monitor.getAttribute("status")
            self.values["vm_type"] = self.variation_monitor.getAttribute("type")

            if self.values["vm_status"] == "on" and not self.values["vm_type"]:
                raise Exception("VM Type required, candidates are: tewma, dleft, simple.")

            self.values["time_window"] = self.variation_monitor.getAttribute("window")

            self.values["life"] = self.variation_monitor.getAttribute("life")

            insert_value = self.variation_monitor.getAttribute("insert_value")

            index_value = ""

            if insert_value in shared.tokens:               
                index_value = shared.tokens[insert_value]
                insert_value = ""

            self.values["index_value"] = index_value
            self.values["insert"] = insert_value
                
        self.metric_id = self.values["metric_id"] = Metric.count

        self.name = self.values["name"]
        

        if self.name in Metric.D:
            raise Exception("Found metrics with same name!")

        Metric.D[self.name] = Metric.count

        Metric.Table.append( (self.name, Metric.count) )

        Metric.count += 1


    def __str__(self):
        
        self.slave = self.values["slave"]

        if self.slave:
            self.values["slave_id"] = Metric.D[self.slave]
        else:
            self.values["slave_id"] = ""

        return \
        """
        <block id="{name}" type="{config}">
            <params>
                <block_type metric_id="{metric_id}" configuration="{config}"/>

                <variation_detector status="{vd_status}" size="20" nhash="8" slave_id="{slave_id}" reset_after="{reset_after}"/>

                <variation_monitor status="{vm_status}" type="{vm_type}" size="20" nhash="8" time_window="{time_window}" life="{life}" index_value="{index_value}" insert_value="{insert}" />

            </params>
        </block>
        """.format(**self.values)


class MetricOp(object):

    attributes = ["id", "vd_update", "vd_get", "vd_set", "vm_update", "vm_get", "vm_set"]

    # _charset = string.letters + string.digits

    # XmlTable = []

    CodeTable = []

    @classmethod
    def reset(self):
        # MetricOp.XmlTable = []
        MetricOp.CodeTable = []

    # @classmethod
    # def getXml(self):        
    #    return "\n".join(MetricOp.XmlTable)

    @classmethod
    def getCode(self, postfix):
        #rcode = "".join( random.choice(MetricOp._charset) for i in xrange(8) )

        MetricOp.LastGenerated = "MetricOp_{0}".format(postfix)

        clist = ", \n\t".join(MetricOp.CodeTable)

        return \
"""
metric_op {0}[] =
{{
  {1},
  {{ -1, 0, 0, 0, 0, 0 }}
}};

""".format( self.LastGenerated, clist )
            

    def __init__(self, metric_op, metric_table):

        values = self.__dict__ 

        # self.__dict__ = values

        for Attr in self.attributes:
            values[Attr] = metric_op.getAttribute(Attr)

        # perform token substitutions

        self.id = metric_table[self.id]

        if self.vd_update != "":
            self.vd_get = self.vd_update
            self.vd_set = self.vd_update

        if self.vm_update != "":
            self.vm_get = self.vm_update
            self.vm_set = self.vm_update

        
        user_fk = set()

        user_fk.add( self.vd_get )
        user_fk.add( self.vd_set )
        user_fk.add( self.vm_get )
        user_fk.add( self.vm_set )


        for key in user_fk:
            if key not in shared.tokens:
                shared.tokens.add_composite(key)
                # MetricOp.XmlTable.append( "<additional_key name=\"{0}\" />".format(key) )

        
        self.vd_get = shared.tokens[ self.vd_get ]
        self.vd_set = shared.tokens[ self.vd_set ]

        self.vm_get = shared.tokens[ self.vm_get ]
        self.vm_set = shared.tokens[ self.vm_set ]

        MetricOp.CodeTable.append( "{{ {id}, {vd_get}, {vd_set}, {vm_set}, {vm_get}, 0  }}".format(**values) )
        
