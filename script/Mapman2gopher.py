from sys import argv
from collections import OrderedDict

inf = open(argv[1], 'r')

if argv[2] == "def":

  first = True

  defs = OrderedDict()

  for line in inf:
    if first:
      first = False
      continue
    line = line.replace("''", 'NULL')
    line = line.replace("'", '')  
    fields = line.strip().split('\t')
    defs[fields[0]] = [fields[1], fields[3]]

  for d in defs:
    print(d, defs[d][0], defs[d][1], sep = '\t')

if argv[2] == "map":

  first = True

  maps = OrderedDict()

  for line in inf:
    if first:
      first = False
      continue
    line = line.replace("''", 'NULL')
    line = line.replace("'", '')  
    fields = line.strip().split('\t')
    if fields[2] == 'NULL':
      continue
    gene = fields[2].split('.')[0].capitalize()
    
    if gene not in maps:
      maps[gene] = set()

    maps[gene].add(fields[0])

  for m in maps:
    print(m, '|'.join(maps[m]), sep = '\t')

