-- Lua script.
p=tetview:new()
p:load_plc("C:/Users/zq/Documents/GitHub/slimeEngine/assets/bunny/bunny.1.smesh")
rnd=glvCreate(0, 0, 500, 500, "TetView")
p:plot(rnd)
glvWait()
