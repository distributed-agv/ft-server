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
elseif not redis.call('GET', 'nonce') then
    local new_nonce = redis.call('GET', 'nonce')
    math.randomseed(tonumber(redis.call('TIME')[1]))
    new_nonce = tostring(math.random(-2^15, -1))
    redis.call('SET', 'nonce', new_nonce)
    redis.call('SET', 'timer', 'TIMER', 'PX', '60000')
    return new_nonce
else 
    return '0'
end