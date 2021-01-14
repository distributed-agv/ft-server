import redis
import json


if __name__ == '__main__':
    config = json.load(open('config.json', 'r'))
    commit_script = open('commit.lua', 'r').read()
    recover_script = open('recover.lua', 'r').read()
    locator_recover_script = open('car-locator/locator_recover.lua', 'r').read()

    r = redis.Redis(config['redis_addr']['host'], config['redis_addr']['port'])
    r.script_load(commit_script)
    r.script_load(recover_script)
    r.script_load(locator_recover_script)
