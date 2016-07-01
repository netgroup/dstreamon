import logging

bm_logger = None

def setup_logging(logfile):
    """\brief Sets up a simple logger and exposes it globally
    \param logfile (\c string) The path to the file to log to
    """
    global bm_logger
    bm_logger = logging.getLogger()
    hdlr = logging.FileHandler(logfile)
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    hdlr.setFormatter(formatter)
    bm_logger.addHandler(hdlr)
    bm_logger.setLevel(logging.INFO)
