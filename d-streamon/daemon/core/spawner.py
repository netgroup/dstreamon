import subprocess

class ProcessSpawner:
    """\brief Static class to Spawn new processes"""

    @staticmethod
    def spawn(cmd, arguments):
        """\brief Spawn a new process with the given arguments
        \param cmd       (\c string)           The command to run (use full paths)
        \param arguments (\c list[string])     Any arguments to give to the new process
        \return          (\c subprocess.Popen) The spawned process
        """
        args = [str(cmd)]
        if len(arguments) > 0:
            for a in arguments:
                args.append(str(a))
        return subprocess.Popen(args)

