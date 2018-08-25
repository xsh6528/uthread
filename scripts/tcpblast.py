#!/usr/bin/env python3

import argparse
import math
import subprocess

BLOCKS_TO_SEND = 1 << 20

def tcpblast(connections):
    blocks = str(math.ceil(float(BLOCKS_TO_SEND) / connections))
    return subprocess.Popen([
        'tcpblast',
        '-4',
        '-a',
        '-c',
        blocks,
        '-r',
        '-s',
        '1KB',
        '-q',
        '127.0.0.1:8000'
    ])

def main():
    '''
    A useful way of using this tool is to benchmark an echo server with just
    a single connection to determine the bandwidth of the underlying link. Then
    increase the number of connections and check that the bandwidth is being
    split uniformly.
    '''
    parser = argparse.ArgumentParser()
    parser.add_argument('connections',
                        type=int,
                        help='how many concurrent tcpblast instances to run')
    args = parser.parse_args()

    if args.connections < 1:
        raise ValueError('cannot have less than one connection!')

    print((
        'WARNING: Use "--threads {}" when running the example echo '
        'server. Press a key to continue...'
    ).format(args.connections))
    input()

    ps = [ tcpblast(args.connections) for _ in range(0, args.connections) ]
    for p in ps:
        p.wait()

if __name__ == '__main__':
    main()