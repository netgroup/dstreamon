class BlockInfo(object):
    """\brief Container class for describing a block to the outside world.
    """

    def __init__(self, type, scheduling_type, gates, params_schema, params_example,\
                 variables, human_desc, short_desc, thread_exclusive=False):
        """\brief Initializes class
        \param type             (\c string)             The block's type (e.g., PFQSource)
        \param scheduling_type  (\c string)             "active"|"passive"|"both"
        \param gates            (\c list[GateInfo])     The block's gates
        \param params_schema    (\c string)             The block's parameters
        \param params_example   (\c string)             Example parameters XML
        \param variables        (\c list[VariableInfo]) The block's variables
        \param human_desc       (\c string)             A human-readable version of what the block does
        \param short_desc       (\c string)             One-line description of block
        \param thread_exclusive (\c bool)               Whether the block needs its own thread pool
        """
        self.__type = type
        self.__scheduling_type = scheduling_type
        self.__gates = gates
        self.__params_schema = params_schema
        self.__params_example = params_example
        self.__variables = variables
        self.__human_desc = human_desc
        self.__short_desc = short_desc
        self.__thread_exclusive = thread_exclusive

    def get_type(self):
        return self.__type

    def scheduling_type(self):
        return self.__scheduling_type

    def get_gates(self):
        return self.__gates

    def get_params_schema(self):
        return self.__params_schema

    def get_params_example(self):
        return self.__params_example

    def get_variables(self):
        return self.__variables

    def get_human_desc(self):
        return self.__human_desc

    def get_short_desc(self):
        return self.__short_desc

    def is_thread_exclusive(self):
        return self.__thread_exclusive

    def __str__(self):
        string = "BlockInfo: type=" + str(self.get_type()) + ", " + \
                            "scheduling_type=" + str(self.scheduling_type()) + ", " + \
                            "is_thread_exclusive=" + str(self.is_thread_exclusive())

        string += "\n"

        for gate in self.get_gates():
            string += "\n" + str(gate)

        string += "\n"

        for var in self.get_variables():
            string += "\n" + str(var)

        string += "\nHuman Description:\n" + str(self.get_human_desc()) + \
                  "\nShort Description:\n" + str(self.get_short_desc()) + \
                  "\nParams Example:\n" + str(self.get_params_example()) + \
                  "\nParams Schema:\n" + str(self.get_params_schema())
        
        return string

class GateInfo(object):
    """\brief Container class for describing a gate to the outside world
    """

    TYPE = ["input", "output"]

    def __init__(self, type, name, msg_type, multiplicity):
        """\brief Initializes class
        \param type         (\c GateInfo.TYPE) The gate's type (e.g., input)
        \param name         (\c string)        The gate's name (e.g., MyInputGate)
        \param msg_type     (\c string)        The gate's message type (e.g., Packet)
        \param multiplicity (\c IntegerRange)  Indicates configurable numbers of gates
        """
        self.__type = type
        self.__name = name
        self.__msg_type = msg_type
        self.__multiplicity = multiplicity

    def get_type(self):
        return self.__type

    def get_name(self):
        return self.__name

    def get_msg_type(self):
        return self.__msg_type

    def get_multiplicity(self):
        return self.__multiplicity

    def __str__(self):
        return "GateInfo: type=" + str(self.get_type()) + ", " + \
                         "name=" + str(self.get_name()) + ", " + \
                         "msg_type=" + str(self.get_msg_type()) + ", " + \
                         "multiplicity=" + str(self.get_multiplicity())


class VariableInfo(object):

    """\brief Container class for accessing block variables to/from the outside world
    """
    ACCESS_TYPE = ["read", "write"]

    def __init__(self, block_name, name, human_desc, access_type, value=None):
        """\brief Initializes class
        \param block_name  (\c string) The parent block (e.g., MyPFQSource)
        \param name        (\c string) The variable's name
        \param human_desc  (\c string) Verbal description of what the value contains
        \param access_type (\c VariableInfo.ACCESS_TYPE) Either read or write
        \param value       (\c void *) The variable's value
        """
        self.__block_name = block_name
        self.__name = name
        self.__type = type
        self.__access_type = access_type
        self.__value = value
        self.__human_desc = human_desc

    def get_block_name(self):
        return self.__block_name

    def get_name(self):
        return self.__name
    
    def get_human_desc(self):
        return self.__human_desc

    def get_access_type(self):
        return self.__access_type

    def get_value(self):
        return self.__value

    def set_value(self, v):
        self.__value = v

    def __str__(self):
        return "VariableInfo: block_name=" + str(self.get_block_name()) + ", " + \
                             "name=" + str(self.get_name()) + ", " + \
                             "human_desc=" + str(self.get_human_desc()) + ", " + \
                             "access_type=" + str(self.get_access_type()) + ", " + \
                             "value=" + str(self.get_value())


class IntegerRange(object):

    """\brief Simple container class for an integer range
    """

    def __init__(self, begin, end):
        """\brief Initializes class
        \param begin (\c int) The range's beginning
        \param end   (\c int) The range's end
        """
        self.__begin = begin
        self.__end = end

    def get_begin(self):
        return self.__begin

    def get_end(self):
        return self.__end

    def __str__(self):
        return "IntegerRange: " + str(self.get_begin()) + "-" + str(self.get_end())



class Block:

    """\brief The main interface to block-related operations on the blockmon executable.
    """

    def __init__(self, name, block_type, comp_id, active, params,\
                       tpool_id, tpool, blockmon, bm_logger):
        """\brief Initializes class, creating a block in the blockmon executable.
        \param name       (\c string)         The block's name
        \param block_type (\c string)         The block's type
        \param comp_id    (\c string)         The composition id
        \param active     (\c bool)           Whether the block is active or not
        \param params     (\c xml.dom.Node)   The block's parameters
        \param tpool_id   (\c string)         The block's thread pool id if active
        \param tpool      (\c ThreadPool)     The block's thread pool if active
        \param bm_logger  (\c logging.logger) The bm logger
        """
        self.__logger = bm_logger
        self.name = str(name)
        self.block_type = str(block_type)
        self.active = active
        self.tpool_id = str(tpool_id)
        self.tpool = tpool
        self.comp_id = str(comp_id)
        self.connections = []
        self.params = str(params)
        self.ingates = {}
        self.outgates = {}
        self.blockmon = blockmon
        self.blockmon.create_block(self.comp_id, \
                                   self.name,\
                                   self.block_type, \
                                   self.active,\
                                   self.params)

        if(active): 
            self.tpool.add_block(self.comp_id, self.name)
        
    def update(self, active, params, tpool_id, tpool):
        """\brief Updates a block
        \param active     (\c bool)           Whether the block is active or not
        \param params     (\c xml.dom.Node)   The block's parameters
        \param tpool_id   (\c string)         The block's thread pool id if active
        \param tpool      (\c ThreadPool)     The block's thread pool if active
        """
        self.params = params
        self.active = active
        if (active and (tpool_id != self.tpool_id)):
            self.tpool.add_block(self.comp_id, self.name)
            self.tpool_id = tpool_id
            self.tpool = tpool
            self.tpool.remove_block(self.comp_id, self.name)
        
        if (self.blockmon.update_block(self.comp_id,\
                                       self.name,\
                                       self.active,\
                                       self.params) < 0):
            self.__logger.info("warning: cannot reconfigure block " + \
                               self.comp_id + ":" + self.name + \
                               ", deleting and rebulding")
            self.remove()
            self.restore()
                              
    def remove(self):
        """\brief Deletes this block from a composition and its thread pool if active
        """
        if (self.active):
            self.tpool.remove_block(self.comp_id, self.name)
        self.blockmon.delete_block(self.comp_id, self.name)

    def restore(self):
        """\brief Re-creates the block and re-adds it to the thread pool if active
        """
        self.blockmon.create_block(self.comp_id,\
                                   self.name,\
                                   self.block_type,\
                                   self.active,\
                                   self.params)
        if (self.active):
            self.tpool.add_block(self.comp_id, self.name)

    def add_connection(self, mygate, otherblock, othergate, direction):
        """\brief Creates a connection between blocks
        \param mygate     (\c string) The source gate
        \param otherblock (\c Block)  The destination block
        \param othergate  (\c string) The destination gate
        \param direction  (\c string) Either in or out
        """
        d = self.ingates if (direction == 'in') else self.outgates
        if (mygate not in d):
            d[mygate] = []
        d[mygate].append((otherblock.comp_id, otherblock.name, othergate))

    def remove_connection(self, mygate, otherblock, othergate, direction):
        """\brief Removes a connection between blocks
        \param mygate     (\c string) The source gate
        \param otherblock (\c Block)  The destination block
        \param othergate  (\c string) The destination gate
        \param direction  (\c string) Either in or out
        """
        d = self.ingates if (mygate in self.ingates.keys()) else self.outgates
        d[mygate].remove((otherblock.comp_id, otherblock.name, othergate))

    def read_var(self, varname):
        """\brief Reads a variable's value
        \param varname (\c string) The variable's name
        \return        (\c string) The variable's value
        """
        ret = self.blockmon.read_block_variable(self.comp_id, self.name, varname)
        if (len(ret) == 0):
            self.__logger.info("warning: read is not supported")
        return ret

    def write_var(self, varname, val):
        """\brief Writes to a variable
        \param varname (\c string) The variable's name
        \param val     (\c string) The value to write
        """
        ret = self.blockmon.write_block_variable(self.comp_id, self.name, varname, val)
        if (ret == -1):
            self.__logger.info("warning: write is not supported")
