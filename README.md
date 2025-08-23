# Lua_curling
A lua binding of my C++ curling library. Enables a user to make some HTTP requests, inside a REPL.


```lua
lua > req = Request.new() \
       :setURL("https://www.example.com") \
       :setMethod(HttpMethod.GET) \
res = req.send(1) \
print(res:toString())
--should output the content of the response
```

## Dependencies
Dependencies are included in this repository for the most part, as curling and sol2 are header-only libs.
Just you would need install the liblua-dev 5.4 and libcurl-dev and your prefered ssl backend.

```ini
liblua-dev=5.4
curling=1.2
sol2=3.0
libcurl 8.5.0
```
