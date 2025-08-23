local Json = {}
Json.__index = Json

--------------------------------------------------------------------
-- 1. Public constructor: Json:new(tbl)
--------------------------------------------------------------------
function Json:new(tbl)
    local self = setmetatable({}, Json)
    self.t     = tbl          -- store the value we’ll serialize
    return self
end

--------------------------------------------------------------------
-- 2. Recursive encoder
--------------------------------------------------------------------
-- local helper – keeps the public table clean
local function encode(v)
    ----------------- primitives -----------------
    local t = type(v)

    if t == "string" then
        -- escape backslashes, quotes & control chars
        local escaped = v:gsub('[\\\"\b\f\n\r\t]', function(c)
            local esc = {
                ['\\'] = '\\\\',
                ['"']  = '\\"',
                ['\b'] = '\\b',
                ['\f'] = '\\f',
                ['\n'] = '\\n',
                ['\r'] = '\\r',
                ['\t'] = '\\t',
            }
            return esc[c]
        end)
        return '"' .. escaped .. '"'

    elseif t == "number" then
        -- NaN / ±inf are not valid JSON – map to null
        if v ~= v or v == math.huge or v == -math.huge then
            return "null"
        else
            return tostring(v)
        end

    elseif t == "boolean" then
        return tostring(v)

    elseif v == nil then
        return "null"

    elseif t == "table" then
        -- decide array vs object
        local isArray = true
        for k in pairs(v) do
            if type(k) ~= "number" then
                isArray = false
                break
            end
        end

        if isArray then
            -- array – indices 1..#v
            local parts = {}
            for i = 1, #v do
                table.insert(parts, encode(v[i]))
            end
            return "[" .. table.concat(parts, ",") .. "]"
        else
            -- object – any key that isn’t a number
            local parts = {}
            for k, val in pairs(v) do
                local key
                if type(k) == "string" then
                    key = '"' .. k .. '"'
                else
                    key = encode(k)     -- for non‑string keys
                end
                table.insert(parts, key .. ":" .. encode(val))
            end
            return "{" .. table.concat(parts, ",") .. "}"
        end
    else
        error("Unsupported type for JSON encoding: " .. t)
    end
end

--------------------------------------------------------------------
-- 3. Public method that returns the JSON string
--------------------------------------------------------------------
function Json:toString()
    return encode(self.t)
end

--------------------------------------------------------------------
-- 4. Return the module
--------------------------------------------------------------------
return Json
