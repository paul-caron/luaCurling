--json encoder
local json = {}

function json.encode(value)
  -- Helper: reject any non‑integer key in an array.
  local function any_nonintkeys(t)
    for k in pairs(t) do
      if type(k) ~= "number" or k ~= math.floor(k) then
        return true
      end
    end
    return false
  end

  local function encode(v)
    local t = type(v)
    if t == "nil" then
      return "null"
    elseif t == "boolean" then
      return tostring(v)
    elseif t == "number" then
      if v ~= v or v == math.huge or v == -math.huge then
        error("JSON does not support NaN or infinite numbers", 2)
      end
      return string.format("%.15g", v)
    elseif t == "string" then
      local out = '"'
      for i = 1, #v do
        local b = string.byte(v, i)
        if b == 34 then          -- "
          out = out .. '\\"'
        elseif b == 92 then      -- \
          out = out .. '\\\\'
        elseif b < 32 then       -- control characters
          out = out .. string.format('\\u%04x', b)
        else
          out = out .. string.char(b)
        end
      end
      return out .. '"'
    elseif t == "table" then
      -- Array?
      if #v > 0 and not any_nonintkeys(v) then      -- see below
        local parts = {}
        for i = 1, #v do
          local val = v[i]
          if val == nil then
            parts[i] = "null"
          else
            parts[i] = encode(val)
          end
        end
        return "[" .. table.concat(parts, ",") .. "]"
      else
        -- object
        local parts = {}
        for k, val in pairs(v) do
          local key_type = type(k)
          if key_type ~= "string" and key_type ~= "number" then
            error("JSON object keys must be strings or numbers", 2)
          end
          if val == nil then
            parts[#parts + 1] = "\"" .. tostring(k) .. "\":null"
          else
            parts[#parts + 1] = "\"" .. tostring(k) .. "\":" .. encode(val)
          end
        end
        return "{" .. table.concat(parts, ",") .. "}"
      end
    else
      error("Unsupported type: " .. t, 2)
    end
  end

  return encode(value)
end

return json

