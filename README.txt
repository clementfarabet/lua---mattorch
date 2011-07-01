
INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install mattorch

USE:
$ lua
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

NOTES:
the package depends on external packages: 'xlua' and 'torch'.
'xlua' is automatically installed by Luarocks, but Torch5 needs
to be installed manually.
Matlab also needs to be installed, and in the path.
