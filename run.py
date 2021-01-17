import redis
import json
import subprocess


if __name__ == '__main__':
    config = json.load(open('config.json', 'r'))
    commit_script = open('commit.lua', 'r').read()
    recover_script = open('recover.lua', 'r').read()
    getlock_script = open('getLock.lua', 'r').read()
    locator_recover_script = open('car-locator/locator_recover.lua', 'r').read()

    r = redis.Redis(config['redis_addr']['host'], config['redis_addr']['port'])
    commit_script_sha = r.script_load(commit_script)
    recover_script_sha = r.script_load(recover_script)
    getlock_script_sha = r.script_load(getlock_script)
    r.script_load(locator_recover_script)

    subprocess.call([
        'build/main',
        str(config['car_num']),
        str(config['row_num']),
        str(config['col_num']),
        config['redis_addr']['host'],
        str(config['redis_addr']['port']),
        config['server_addr']['host'],
        str(config['server_addr']['port']),
        commit_script_sha,
        recover_script_sha,
        getlock_script_sha,
    ])