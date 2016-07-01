class ReturnValue(object):
    """\brief Container for a return value for a daemon task or command. If the
              the code is set to 0 the operation succeeded and the return value is
              available. Any other code indicates an error.
              [NOTE: we need to define other values, ala HTTP]
    """
    CODE_SUCCESS = 0
    CODE_FAILURE = -1
    SIMPLE_STR_VARS = ["code", "msg", "value"]

    def __init__(self, code=None, msg=None, value=None):
        """\brief Initializes class
        \param code  (\c int)    The numeric code
        \param msg   (\c string) The message, useful in case of error
        \param value (\c void)   The actual return value
        """
        self.__code = code
        self.__msg = msg
        self.__value = value

    def get_code(self):
        """\brief Retrieves the numeric code
        \return (\c int) The numeric code
        """
        return self.__code

    def get_msg(self):
        """\brief Retrieves the message
        \return (\c string) The message
        """
        return self.__msg

    def get_value(self):
        """\brief Retrieves the actual return value
        \return (\c void) The actual return value
        """
        return self.__value

    def __str__(self):
        string = str(self.__class__.__name__) + ": "
        for str_var in self.SIMPLE_STR_VARS:
            string += str_var + "=" + str(eval('self.get_' + str_var)()) + ","
        string = string[:len(string) - 1]
        return string
