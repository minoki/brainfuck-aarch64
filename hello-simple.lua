local current = 0
function put(c)
  local value = c:byte()
  if current > value then
    io.write(string.rep("-", current - value))
  else
    io.write(string.rep("+", value - current))
  end
  current = value
  io.write(".")
  io.write("\n")
end
put("H")
put("e")
put("l")
put("l")
put("o")
put(" ")
put("w")
put("o")
put("r")
put("l")
put("d")
put("!")
put("\n")

