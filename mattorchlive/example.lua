
luaprefix = '/home/cf65/lua-local'
package.path='./?.lua;'..luaprefix..'/share/lua/5.1/?.lua;'..luaprefix..'/share/lua/5.1/?/init.lua;'..luaprefix..'/lib/lua/5.1/?.lua;'..luaprefix..'/lib/lua/5.1/?/init.lua'
package.cpath='./?.so;./?.dylib;'..luaprefix..'/lib/lua/5.1/?.so;'..luaprefix..'/lib/lua/5.1/?.dylib;'..luaprefix..'/lib/lua/5.1/loadall.so'..luaprefix..'/lib/lua/5.1/loadall.dylib'

require 'torch'

transpose = function(img)
               print('from Lua: receiving img of size: ', img:size())
               if img:nDimension() == 3 then
                  print('tranposing image')
                  return img:transpose(2,3)
               elseif img:nDimension() == 2 then
                  print('tranposing image')
                  return img:transpose(1,2)
               else
                  error('require a matrix with 2 or 3 dimensions')
               end
            end

print('loaded example.lua with success!')

