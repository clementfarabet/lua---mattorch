DEPENDENCIES:
Torch7 (www.torch.ch)

INSTALL:
$ torch-rocks install mattorch

USE:
$ torch
> require 'mattorch'
-- save:
> tensor1 = torch.Tensor(...)
> tensor2 = torch.Tensor(...)
> tensor3 = torch.Tensor(...)
> mattorch.save('output.mat', tensor1)
-- OR
> list = {myvar = tensor1, othervar = tensor2, thisvar = tensor3}
> mattorch.save('output.mat', list) ]]

-- load:
> loaded = mattorch.load('input.mat')
