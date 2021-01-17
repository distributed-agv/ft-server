local car_id = ARGV[1]
local seq = ARGV[2]
local name_1 = ARGV[3]
local name_2 = ARGV[4]
local name_3 = ARGV[5]
local name_4 = ARGV[6]
local name_5 = ARGV[7]


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

seq = tonumber(seq)

local last_seq = tonumber(redis.call('GET', 'seq:'..car_id))
if seq + 1 == last_seq then
    return redis.call('GET', 'step_code:'..car_id)
elseif seq ~= last_seq then
    return '0'
end


if(name_1~="")then
    redis.call('HSETNX', 'owner_map', name_1,car_id)
end
if(name_2~="")then
    redis.call('HSETNX', 'owner_map', name_2,car_id)
end
if(name_3~="")then
    redis.call('HSETNX', 'owner_map', name_3,car_id)
end
if(name_4~="")then
    redis.call('HSETNX', 'owner_map', name_4,car_id)
end
if(name_5~="")then
    redis.call('HSETNX', 'owner_map', name_5,car_id)
end

return '7'