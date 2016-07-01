class ThreadPool:

    """\brief Container class for a thread pool
    """
    
    def __init__(self, poolnode, blockmon):
        """\brief Initializes class, adding a thread pool to the blockmon executable.
        \param poolnode (\c Node)   An xml node list representing the thread pool info
        \param blockmon (\c module) The module for the blockmon executable
        """
        self.blocks = []
        self.__blockmon = blockmon
        self.name = str(poolnode.attributes['id'].value)
        self.nthreads = poolnode.attributes['num_threads'].value
        self.__blockmon.add_thread_pool(str(poolnode.toxml()))

    def add_block(self, comp_id, name):
        """\brief Adds a block to the thread pool
        \param comp_id (\c string) The composition id
        \param name    (\c string) The block's name
        """
        self.blocks.append((comp_id, name))
        self.__blockmon.add_block_to_thread_pool(comp_id, name, self.name)

    def remove_block(self, comp_id, name):
        """\brief Removes a block from the thread pool
        \param comp_id (\c string) The composition id
        \param name    (\c string) The block's name
        """
        self.blocks.remove((comp_id, name))
        self.__blockmon.remove_block_from_thread_pool(comp_id, name, self.name)

    def __del__(self):
        """\brief Removes itself from the blockmon executable
        """
        if(len(self.blocks) > 0):
            raise Exception('removing thread pool having assigned tasks')
        self.__blockmon.remove_thread_pool(self.name)
