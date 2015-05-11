from tree import Tree
from copy import deepcopy
import sys
import NewTree
import argparse
import os
import re
import sys
import codecs

parser = argparse.ArgumentParser(description='')
parser.add_argument('gr_or_gp', type=int, metavar='',help='Generate Rules -- 0, Generate Parts -- 1, Generate CONLL from Phrase Treebank -- 2.')
parser.add_argument('--inputf', type=str, metavar='', help='The input file. For most of the time, the train set of the Treebank.')
parser.add_argument('--rulef', type=str, metavar='', help='The input rule file in mode 1.')
parser.add_argument('--unary_collapse', action='store_true', help='Collapse the unary rules in the phrase structure tree, default not.')
parser.add_argument('--language', type=str, metavar='', help='chn for Chinese, default English')
parser.add_argument('--backoff_rule',action='store_true',help='Generate Backoff rules. Default True.')

A = parser.parse_args()

use_back_off_rule = A.backoff_rule
unary_collapse = A.unary_collapse
language_setting = A.language

def generate_rule(treebank_file):
    # if you use unicode here, there is a bug...
    f = open(treebank_file, "r")
    pos_set = set([])
    full_rule_set = set([])
    s_ind = 0
    for sentence in f:
        if language_setting == "chn":
            sentence = sentence.decode('utf-8')
        s_ind += 1
        if s_ind % 100 == 0:
            sys.stderr.write(str(s_ind) + "..")
        tree = Tree.fromstring(sentence, remove_empty_top_bracketing=False)
        preterminals = [t.label() for t in tree.subtrees(lambda t: t.height() == 2)]
        pos_set.update(preterminals)

        # First, collapse the unary, notice that the POS tags should not be affected
        if unary_collapse:
            NewTree.collapse_unary(tree)
        bt = NewTree.get_binarize_lex(tree)
        # Extract rules from the tree
        rule_set = NewTree.generate_rules(bt)

        # Add them to the full set
        for sr in rule_set:
            full_rule_set.add(sr)
    sys.stderr.write("\n")
    f.close()

    # print core_pos_set
    # Generate the back-off rules
    backoff_rule_set = set([])
    for r in full_rule_set:
        args = r.split(" ")

        for i in xrange(1, len(args)-1):
            if args[i] in pos_set:
                args_copy = deepcopy(args)
                args_copy[i] = "BPOS|"
                backoff_rule_set.add(" ".join(args_copy))

    ind = 0
    for r in full_rule_set:
        print str(ind) + " " + r
        ind += 1
    if use_back_off_rule:
        for r in backoff_rule_set:
            print str(ind) + " " + r
            ind += 1
        for pos in pos_set:
            print str(ind) + " 1 BPOS| " + pos
            ind += 1


def generate_part(treebank_file, rule_file):
    # This generate the gold parts file for the use of C++
    rule_dic = read_rule_file(rule_file)
    f = open(treebank_file, "r")
    s_ind = 0

    for sentence in f:
        if language_setting == "chn":
            sentence = sentence.decode('utf-8')
        s_ind += 1
        if s_ind % 100 == 0:
            sys.stderr.write(str(s_ind) + "..")
        parts = []
        t = Tree.fromstring(sentence, remove_empty_top_bracketing=False)
        if unary_collapse:
            NewTree.collapse_unary(t)
        bt = NewTree.get_binarize_lex(t)
        for pos in bt.treepositions(order='postorder'):
            nt = bt[pos]
            if isinstance(nt, str) or isinstance(nt, unicode):
                continue
            elif nt.height() == 2:
                continue
            else:
                info = NewTree.get_span_info(nt,rule_dic)
                parts.append(info)

        work_tree = deepcopy(t)
        NewTree.lexLabel(work_tree)
        parent_dic, dep_label_set = NewTree.getParentDic(work_tree)

        print len([item for item in parts if item != None])
        parent_list = []
        label_list = []
        for ind in xrange(1, (len(t.leaves()) + 1)):
            p = str(int(parent_dic[str(ind)]) - 1)
            parent_list.append(p)
        for ind in xrange(1, (len(t.leaves()) + 1)):
            l = dep_label_set[str(ind)]
            label_list.append(l)
        for p in parts:
            if p != None:
                print " ".join(p)
            else:
                pass
    sys.stderr.write("\n")
    f.close()

def read_rule_file(rulef):
    rule_dic = {}
    f = open(rulef, "r")
    for line in f:
        l = line.split(' ', 1)
        rule_dic[l[1].strip()] = l[0]
    f.close()
    return rule_dic

def read_corpus(filename):
    f = open(filename, "r")
    corpus = []
    sentence = []
    for line in f:
        if language_setting == "chn":
            line = line.decode('utf-8')
        if line[:-1] == "":
            corpus.append(sentence)
            sentence = []
            continue
        else:
            line = line[:-1]
            cline = line.split("\t")
            sentence.append(cline)
    f.close()
    return corpus

def generate_conll(inputf):
    f = open(inputf, "r")
    s_ind = 0
    for sentence in f:
        if language_setting == "chn":
            sentence = sentence.decode('utf-8')
        s_ind += 1
        if s_ind % 100 == 0:
            sys.stderr.write(str(s_ind) + "..")
        t = Tree.fromstring(sentence, remove_empty_top_bracketing=False)
        deps = NewTree.generateDep(t)
        NewTree.print_conll_lines(deps,sys.stdout)
        sys.stdout.write("\n")
    f.close()


if __name__ == '__main__':

    NewTree.HORZMARKOV = 0
    NewTree.VERTMARKOV = 0

    if A.language == "chn":
        NewTree.LANGUAGE = "chn"
        sys.stdout = codecs.getwriter('utf-8')(sys.stdout)
        sys.stderr = codecs.getwriter('utf-8')(sys.stderr)
    else:
        NewTree.LANGUAGE = "eng"

    if A.gr_or_gp == 0:
        generate_rule(A.inputf)
    elif A.gr_or_gp == 1:
        generate_part(A.inputf,A.rulef)
    else:
        generate_conll(A.inputf)
