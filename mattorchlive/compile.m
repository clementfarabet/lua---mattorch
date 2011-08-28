
% home
getenv HOME;
home = ans;
hlua = ['-I' home '/lua-local/include'];
hTH = [hlua '/TH'];

% compile
mex('example.c', 'libmattorchlive.so', hlua, hTH)
