
from xml.dom import minidom


class Fields(dict):

    def __init__(self, filepath):
        super(Fields, self).__init__()

        self._max = 0
        self._additionals = []

        self[""] = -1
        self["null"] = -1

        source = minidom.parse(filepath)

        fields = source.getElementsByTagName('field')

        for field in fields:
            
            name = field.getAttribute('name')
            value = field.getAttribute('id')

            num_value = int(value)

            self[name] = num_value

            if num_value > self._max:
                self._max = num_value

        self._max += 1

    def add(self, token):
        self[token] = self._max

        self._max += 1

    def add_composite(self, token):
        self.add(token)

        self._additionals.append("<additional_key name=\"{0}\" />".format(token))

    def names(self):

        ls = []

        for key in self:
            if key != "" and key != "null":
                ls.append(key)

        return ls

    def toxml(self):

        return "\n".join( """<field name="{0}" id="{1}" />""".format(k, v) for k, v in self.iteritems() )

    def getAdditionalXml(self):
        return "\n".join(self._additionals)
