from sys import argv

file_in = argv[1]

clusters = [l.strip().split()[1] for l in open(file_in, 'r')]

s = set()

for c in clusters:
  cx = ""
  for i in list(c):
    if i == ":":
      s.add(cx)
    cx += i
  s.add(cx)    

for c in s:
  print(c,"Leaf terminal cluster " + c, "Leaf terminal cluster " + c, sep = '\t')