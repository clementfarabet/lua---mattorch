----------------------------------------------------------------------
--
-- Copyright (c) 2010 Clement Farabet
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     matlab - this bit of code is a simple wrapper around the matlab
--              library (load/save functions for now)
--
-- history: 
--     June 30, 2011, 6:15PM - creation - Clement Farabet
----------------------------------------------------------------------

require 'xlua'
require 'torch'
require 'libmattorch'

------------------------------------------------------------
-- helps
--
local help = {
---
load = [[Loads a .mat file into a Lua table. 
Each mex Array is converted into a torch.Tensor.
A table with all the loaded variables is returned:
  {varname1 = var1, varname2 = var2, ... } ]]
,
save = [[Exports variables to a .mat file.
Supported now:
  > tensor1 = torch.DoubleTensor(...)
  > tensor2 = torch.DoubleTensor(...)
  > tensor3 = torch.DoubleTensor(...)
  > mattorch.save('output.mat', tensor1)
  > -- OR
  > list = {myvar = tensor1, othervar = tensor2, thisvar = tensor3}
  > mattorch.save('output.mat', list) ]]
}

------------------------------------------------------------
-- create package
--
mattorch = {}

-- load
mattorch.load = function(path)
                 if not path then
                    xlua.error('please provide a path','mattorch.load',help.load)
                 end
                 return libmattorch.load(path)
              end

-- save
mattorch.save = function(path,vars)
                 if not path or not vars then
                    xlua.error('please provide a path','mattorch.save',help.save)
                 end
                 if type(vars) == 'userdata' and torch.typename(vars) == 'torch.DoubleTensor' then
                    local tensor = torch.Tensor():resizeAs(vars):copy(vars)
                    libmattorch.saveTensor(path,tensor)

                 elseif type(vars) == 'table' then
                    for i,v in ipairs(vars) do
                       if v then
                          xlua.error('can only export table of named variables, e.g. {x=..., y=...}',
                                     'mattorch.save',help.save) 
                       end
                    end
                    for _,v in pairs(vars) do
                       if type(v) ~= 'userdata' or torch.typename(v) ~= 'torch.DoubleTensor' then 
                          xlua.error('can only export table of torch.DoubleTensor',
                                     'mattorch.save',help.save)
                       end
                    end
                    libmattorch.saveTable(path,vars)

                 else
                    xlua.error('cannot export given variables','mattorch.save',help.save)
                 end
              end

-- save
mattorch.saveAscii = function(path,var)
                        if type(var) == 'userdata' and torch.typename(var) == 'torch.DoubleTensor' then
                           local file = torch.DiskFile(path,'w')
                           libmattorch.saveTensorAscii(file, var)
                           file:close()
                        else
                           xlua.error('can only export 1D or 2D DoubleTensors','mattorch.saveAscii')
                        end
                     end

-- return package
return mattorch
