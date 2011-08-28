
require 'image'

transpose = function(img)
               if img:nDimension() == 3 then
                  return img:transpose(2,3)
               elseif img:nDimension() == 2 then
                  return img:transpose(1,2)
               else
                  error('require a matrix with 2 or 3 dimensions')
               end
            end
