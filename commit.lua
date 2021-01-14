local car_id = ARGV[1]
local seq = ARGV[2]
local step_code = ARGV[3]
local acquiring_pos = ARGV[4]
local releasing_pos = ARGV[5]

if not redis.call('GET', 'status') then
    local nonce = redis.call('GET', 'nonce')
    if not nonce then
        math.randomseed(tonumber(redis.call('TIME')[1]))
        nonce = tostring(math.random(-2^15, -1))
        redis.call('SET', 'nonce', nonce)
        redis.call('SET', 'timer', 'TIMER', 'PX', '60000')
    end
    return nonce
end

if redis.call('SISMEMBER', 'car_ids', car_id) == 0 then
    return '0'
end

seq = tonumber(seq)
local last_seq = tonumber(redis.call('GET', 'seq:'..car_id))
if seq + 1 == last_seq then
    return redis.call('GET', 'step_code:'..car_id)
elseif seq ~= last_seq then
    return '0'
end
redis.call('INCR', 'seq:'..car_id)

local owner_id = redis.call('HGET', 'owner_map', acquiring_pos)
if owner_id and owner_id ~= car_id then
    step_code = '0'
else
    redis.call('HSET', 'owner_map', acquiring_pos, car_id)
end

owner_id = redis.call('HGET', 'owner_map', releasing_pos)
if owner_id and owner_id == car_id then
    redis.call('HDEL', 'owner_map', releasing_pos)
end

redis.call('SET', 'step_code:'..car_id, step_code)
return step_code
