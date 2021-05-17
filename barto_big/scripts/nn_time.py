"""
Compute time spend in Deepsymbol
Usage : python nn_time.py <file>.log <timefile>
"""

import sys
import os
import subprocess
from datetime import datetime

def main(outfile):
    """
    Assumes a "logfile" is created and reads from it
    """
    elapsedTime = 0
    with open("logfile", "r") as fp:
        with open(outfile, "a+") as op:
            for line in fp:
                l = [x for x in line.strip().split()]
                if("deepsymbol" in l or "eran" in l):
                    l2 = [x for x in fp.readline().strip().split()]

                    #Now we get both datetime objects
                    t1 = datetime.strptime("{} {} {} {}".format(l[6], l[7], l[8], l[9]), '%b %d %H:%M:%S %Y')
                    t2 = datetime.strptime("{} {} {} {}".format(l2[7], l2[8], l2[9], l2[10]), '%b %d %H:%M:%S %Y')
                    eT = t2 - t1
                    elapsedTime = elapsedTime + eT.total_seconds()

            op.write("{}\n".format(elapsedTime))
                    
    
if __name__ == "__main__":
    # input_log = sys.argv[1]
    outfile = sys.argv[2]
    # subprocess.call(["sed -n 's/^\(Reached crab intrinsic.*\)/\1/p' {} > logfile".format(input_log)], shell=True)
    main(outfile)
