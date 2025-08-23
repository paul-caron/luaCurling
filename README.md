# lua_curling
A lua binding of my C++ curling library. Enables a user to make some HTTP requests, inside a REPL.


```lua
lua > req = Request.new() \
       :setURL("https://www.example.com") \
       :setMethod(HttpMethod.GET) \
res = req.send(1) \
print(res:toString())
--should output the content of the response
```

## dependencies
Dependencies are included in this repository for the most part, as curling and sol2 are header-only libs.
Just you would need install the liblua-dev 5.4.
```ini
liblua-dev=5.4
curling=1.2
sol2=3.0
```
