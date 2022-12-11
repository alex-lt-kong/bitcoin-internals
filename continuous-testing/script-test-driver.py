import json
import logging
import requests
import subprocess
import sys
import time


lgr = logging.getLogger()
handler = logging.StreamHandler(sys.stdout)
handler.setFormatter(
    fmt=logging.Formatter('%(asctime)s | %(name)s | %(levelname)s | %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
))
lgr.addHandler(handler)
lgr.setLevel(logging.INFO)

url_latestblock = 'https://blockchain.info/latestblock'
lgr.info(f'Requesting latest block through [{url_latestblock}]')
resp = requests.get(url_latestblock)
height = int(resp.json()['height'])
lgr.info(f"Latest block fetched, it's height is {height:,}")

url_block_by_height = f'https://blockchain.info/block-height/{height}'
lgr.info(f'Requesting transactions in the latest block through [{url_block_by_height}]')
resp = requests.get(url_block_by_height)
latest_block = resp.json()['blocks'][0]
txes = latest_block['tx']
lgr.info(f"All transactions fetched, count: {len(txes):,}")

test_program = './script-test.out'
for i in range(len(txes)):
    lgr.info(f'[{i+1}/{len(txes)}] tx hash: {txes[i]["hash"]}')
    # resp = requests.get(f'https://blockstream.info/api/tx/{txes[i]["hash"]}/hex')
    #slgr.info(f'[{i+1:,}/{len(txes):,}] tx bytes: {resp.text}')
    for j in range(len(txes[i]['inputs'])):
        tx_in = txes[i]['inputs'][j]
        if tx_in['script'] == '':
            lgr.info(f'[{i+1}/{len(txes)}] script is empty, skipped')
            continue
        test_cmd = [test_program, str(tx_in['script'])]        
        p = subprocess.Popen(test_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        stdout, stderr = p.communicate()
        retval = p.wait()
        if stdout.decode('utf8') == 'okay\n' and stderr is None and retval == 0:
            lgr.info(f'[{i+1}/{len(txes)}] {j+1}th input: test program [{test_program}] reports okay.')
        else:
            raise ValueError(f'Test program [{test_program}] reports error. stdout: {stdout}, stderr: {stderr}');

lgr.info(f'All scripts are tested by [{test_program}] and no errors are reported')