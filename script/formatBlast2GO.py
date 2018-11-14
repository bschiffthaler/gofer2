import re
import argparse

parser = argparse.ArgumentParser(description="""
Tiny helper tool to format Blast2GO output for Gofer2 (or more generally 
format B2G into a delimited file).
                                 """)

parser.add_argument("gene_re", help="RegExp describing gene pattern")
parser.add_argument("term_re", help="RegExp describing term pattern")
parser.add_argument("in_file", help="Input file")

parser.add_argument("-d", "--delimiter", 
                    help="Optional output delimiter (default: \\t)", 
                    default="\t")
parser.add_argument("-s", "--separator", 
                    help="Optional term separator (default: |)", 
                    default="|")

args = parser.parse_args()

gene_map = re.compile(args.gene_re)
term_map = re.compile(args.term_re)

with open(args.in_file, "r") as inf:
  for line in inf:
    gene = gene_map.search(line)
    terms = term_map.findall(line)
    if gene and terms:
      print(gene.group(0), args.separator.join(list(set(terms))), 
            sep=args.delimiter)

