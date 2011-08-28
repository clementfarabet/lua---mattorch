
require "torch"

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
