import redis
import json
import subprocess


if __name__ == '__main__':
    config = json.load(open('config.json', 'r'))
    commit_script = open('commit.lua', 'r').read()
    recover_script = open('recover.lua', 'r').read()

    r = redis.Redis(config['redis_addr']['host'], config['redis_addr']['port'])
    commit_script_sha = r.script_load(commit_script)
    recover_script_sha = r.script_load(recover_script)

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
