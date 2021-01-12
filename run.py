import redis
import json
import subprocess
import hashlib


if __name__ == '__main__':
    config = json.load(open('config.json', 'r'))
    commit_script = open('commit.lua', 'r').read().encode('utf8')
    recover_script = open('recover.lua', 'r').read().encode('utf8')
    commit_script_sha = hashlib.sha1(commit_script).hexdigest()
    recover_script_sha = hashlib.sha1(recover_script).hexdigest()

    subprocess.call([
        'build/server',
        str(config['car_num']),
        str(config['row_num']),
        str(config['col_num']),
        config['redis_addr']['host'],
        str(config['redis_addr']['port']),
        config['server_addr']['host'],
        str(config['server_addr']['port']),
        commit_script_sha,
        recover_script_sha,
    ])
