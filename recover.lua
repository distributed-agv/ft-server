local car_id = ARGV[1]
local nonce = ARGV[2]
local cur_pos = ARGV[3]
local car_num = ARGV[4]

if redis.call('GET', 'status') then
    return '5'
elseif nonce == redis.call('GET', 'nonce') then
    redis.call('SET', 'seq:'..car_id, '0')
    redis.call('HSET', 'owner_map', cur_pos, car_id)
    redis.call('SADD', 'car_ids', car_id)
    if redis.call('SCARD', 'car_ids') == tonumber(car_num) then
        redis.call('SET', 'status', 'NORMAL')
        return '5'
    end
    return nonce
else
    return '0'
end