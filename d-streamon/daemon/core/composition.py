from core.threadpool import ThreadPool
from core.block import Block

class CompositionManager:

    """\brief Manages a blockmon composition
    """

    def __init__(self, comp_id, blockmon, bm_logger):
        """\brief Initializes class
        \param comp_id   (\c string)         The composition id
        \param blockmon  (\c module)         The blockmon executable module
        \param bm_logger (\c logging.logger) The logger
        """
        self.__logger = bm_logger
        self.__comp_id = str(comp_id)
        self.__blocks = {}         # dict block id -> Block
        self.__thread_pools = {}   # dict pool id  -> ThreadPool
        self.__blockmon = blockmon
        self.__blockmon.add_composition(self.__comp_id)

    def install(self, xmlnode):
        """\brief Installs a blockmon composition
        \param xmlnode (\c xml.dom.Node) The blockmon composition
        """
        for n in xmlnode.getElementsByTagName("threadpool"):
            self.__create_pool(n)

        for n in xmlnode.getElementsByTagName("block"):
            self.__create_block(n)

        for n in xmlnode.getElementsByTagName("connection"):
            self.__create_connection(n)

        self.__logger.info(self.__comp_id + ' : installed')

    def reconfigure(self, xmlnodes):
        """\brief Reconfigures a blockmon composition
        \param xmlnodes (\c xml.dom.Node) The blockmon composition
        """
        for xmlnode in xmlnodes.getElementsByTagName("delete"):
            for n in xmlnode.getElementsByTagName("connection"):
                self.__delete_connection(n)

            for n in xmlnode.getElementsByTagName("block"):
                self.__delete_block(n)

            for n in xmlnode.getElementsByTagName("threadpool"):
                self.__delete_pool(n)

        for xmlnode in xmlnodes.getElementsByTagName("add"):

            for n in xmlnode.getElementsByTagName("block"):
                self.__create_block(n)

            for n in xmlnode.getElementsByTagName("connection"):
                self.__create_connection(n)

            for n in xmlnode.getElementsByTagName("threadpool"):
                self.__create_pool(n)

        for xmlnode in xmlnodes.getElementsByTagName("reconf"):
            
            for n in xmlnode.getElementsByTagName("block"):
                self.__update_block(n)

        self.__logger.info(self.__comp_id + ' : updated')

    def remove(self):
        """\brief Removes the currently installed blockmon composition
        """
        for i in self.__blocks.values():
            i.remove()
        self.__blockmon.delete_composition(self.__comp_id)

    def read_block_var(self, blockname, varname):
        """\brief Reads a block variable's value
        \param blockname (\c string) The block's name
        \param varname   (\c string) The variable's name
        \reuturn         (\c string) The variable's value
        """
        return self.__blocks[blockname].read_var(varname)

    def write_block_var(self, blockname, varname, val):
        """\brief Writes a value to a block variable
        \param blockname (\c string) The block's name
        \param varname   (\c string) The variable's name
        \param val       (\c string) The value to write
        """
        self.__blocks[blockname].write_var(varname, val)

    def __create_block(self, blocknode):
        """\brief Creates a block
        \param blocknode (\c xml.dom.Node) The block in xml form
        """
        block_name = blocknode.attributes['id'].value
        block_type = blocknode.attributes['type'].value
        active = False
        tpool_id = ""
        if blocknode.attributes.has_key('sched_type'):
            active = True if (blocknode.attributes['sched_type'].value == 'active') else False
            if active:
                tpool_id = blocknode.attributes['threadpool'].value

        params = blocknode.getElementsByTagName('params')
        params = params[0].toxml()
        tpool = None
        if (active):
            tpool = self.__thread_pools[str(tpool_id)]
        self.__blocks[block_name] = Block(block_name,\
                                          block_type,\
                                          self.__comp_id,\
                                          active,\
                                          params,\
                                          tpool_id,\
                                          tpool,\
                                          self.__blockmon,\
                                          self.__logger)

    def __delete_block(self, blocknode):
        """\brief Deletes a block
        \param blocknode (\c xml.dom.Node) The block in xml form
        """
        block_name = blocknode.attributes['id'].value
        self.__blocks[block_name].remove()
        del self.__blocks[block_name]

    def __update_block(self, blocknode):
        """\brief Updates a block
        \param blocknode (\c xml.dom.Node) The block in xml form
        """
        block_name = blocknode.attributes['id'].value
        active = False
        tpool_id = ""
        if blocknode.attributes.has_key('sched_type'):
            active = True if (blocknode.attributes['sched_type'].value == 'active') else False
            tpool_id = blocknode.attributes['threadpool'].value

        params = blocknode.getElementsByTagName('params')
        params = str(params[0].toxml())
        tpool = None
        if (active):
            tpool = self.__thread_pools[str(tpool_id)]
        self.__blocks[block_name].update(active, params, tpool_id, tpool)

    def __create_pool(self, poolnode):
        """\brief Creates a thread pool
        \param poolnode (\c xml.dom.Node) The thread pool in xml form
        """
        self.__thread_pools[poolnode.attributes['id'].value] = ThreadPool(poolnode,\
                                                                          self.__blockmon)
    def __delete_pool(self, poolnode):
        """\brief Deletes a thread pool
        \param poolnode (\c xml.dom.Node) The thread pool in xml form
        """
        del self.__thread_pools[poolnode.attributes['id'].value]

    def __create_connection(self, connection_node):
        """\brief Creates a connection between two blocks
        \param connection_node (\c xml.dom.Node) The connection in xml form
        """
        bsource = str(connection_node.attributes['src_block'].value)
        gsource = str(connection_node.attributes['src_gate'].value)
        bdst = str(connection_node.attributes['dst_block'].value)
        gdst = str(connection_node.attributes['dst_gate'].value)
        self.__connect_blocks(self.__blocks[bsource], gsource, self.__blocks[bdst], gdst)

    def __delete_connection(self, connection_node):
        """\brief Deletes a connection between two blocks
        \param connection_node (\c xml.dom.Node) The connection in xml form
        """
        bsource = str(connection_node.attributes['src_block'].value)
        gsource = str(connection_node.attributes['src_gate'].value)
        bdst = str(connection_node.attributes['dst_block'].value)
        gdst = str(connection_node.attributes['dst_gate'].value)
        self.__disconnect_blocks(self.__blocks[bsource], gsource, self.__blocks[bdst], gdst)

    def __connect_blocks(self, b1, g1, b2, g2):
        """\brief Connects two blocks
        \param b1 (\c Block)  The source block
        \param g1 (\c string) The source gate
        \param b2 (\c Block)  The destination block
        \param g2 (\c string) The destination gate
        """
        self.__blockmon.create_connection(b1.comp_id,\
                                          b1.name,\
                                          g1,\
                                          b2.comp_id,\
                                          b2.name,\
                                          g2)
        b1.add_connection(g1, b2, g2, 'out')
        b2.add_connection(g2, b1, g1, 'in')

    def __disconnect_blocks(self, b1, g1, b2, g2):
        """\brief Disconnects two blocks
        \param b1 (\c Block)  The source block
        \param g1 (\c string) The source gate
        \param b2 (\c Block)  The destination block
        \param g2 (\c string) The destination gate
        """
        self.__blockmon.delete_connection(b1.comp_id, \
                                          b1.name, \
                                          g1, \
                                          b2.comp_id, \
                                          b2.name, \
                                          g2)
        b1.remove_connection(g1, b2, g2, 'out')
        b2.remove_connection(g2, b1, g1, 'in')
