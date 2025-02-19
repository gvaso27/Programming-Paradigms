#!/usr/bin/env python

import random # for seed, random
import sys    # for stdout



################################### TEST PART ##################################
################################################################################

# Tests align strands and scores
# Parameters types:
#    score          =  int   example: -6
#    plusScores     = string example: "  1   1  1"
#    minusScores    = string example: "22 111 11 "
#    strandAligned1 = string example: "  CAAGTCGC"
#    strandAligned2 = string example: "ATCCCATTAC"
#
#   Note: all strings must have same length
def test(score, plusScores, minusScores, strandAligned1, strandAligned2):
    print("\n>>>>>>START TEST<<<<<<")

    if testStrands(score, plusScores, minusScores, strandAligned1, strandAligned2):
        sys.stdout.write(">>>>>>>Test SUCCESS:")
        sys.stdout.write("\n\t\t" + "Score: "+str(score))
        sys.stdout.write("\n\t\t+ " + plusScores)
        sys.stdout.write("\n\t\t  " + strandAligned1)
        sys.stdout.write("\n\t\t  " + strandAligned2)
        sys.stdout.write("\n\t\t- " + minusScores)
        sys.stdout.write("\n\n")
    else:
        sys.stdout.write("\t>>>>!!!Test FAILED\n\n")


# converts character score to int
def testScoreToInt(score):
    if score == ' ':
        return 0
    return int(score)


# computes sum of scores
def testSumScore(scores):
    result = 0
    for ch in scores:
        result += testScoreToInt(ch)
    return result


# test each characters and scores
def testValidateEach(ch1, ch2, plusScore, minusScore):
    if ch1 == ' ' or ch2 == ' ':
        return plusScore == 0 and minusScore == 2
    if ch1 == ch2:
        return plusScore == 1 and minusScore == 0
    return plusScore == 0 and minusScore == 1


# test and validates strands
def testStrands(score, plusScores, minusScores, strandAligned1, strandAligned2):
    if len(plusScores) != len(minusScores) or len(minusScores) != len(strandAligned1) or len(strandAligned1) != len(
            strandAligned2):
        sys.stdout.write("Length mismatch! \n")
        return False

    if len(plusScores) == 0:
        sys.stdout.write("Length is Zero! \n")
        return False

    if testSumScore(plusScores) - testSumScore(minusScores) != score:
        sys.stdout.write("Score mismatch to score strings! TEST FAILED!\n")
        return False
    for i in range(len(plusScores)):
        if not testValidateEach(strandAligned1[i], strandAligned2[i], testScoreToInt(plusScores[i]),
                                testScoreToInt(minusScores[i])):
            sys.stdout.write("Invalid scores for position " + str(i) + ":\n")
            sys.stdout.write("\t char1: " + strandAligned1[i] + " char2: " +
                             strandAligned2[i] + " +" + str(testScoreToInt(plusScores[i])) + " -" +
                             str(testScoreToInt(minusScores[i])) + "\n")
            return False

    return True

######################## END OF TEST PART ######################################
################################################################################


# Computes the score of the optimal alignment of two DNA strands.
def findOptimalAlignment(strand1, strand2):
    maxLength = max(len(strand1), len(strand2))
    strand1 = strand1.ljust(maxLength)
    strand2 = strand2.ljust(maxLength)

    score = 0
    plusScores = ""
    minusScores = ""

    for ch1, ch2 in zip(strand1, strand2):
        if ch1 == " " or ch2 == " ":
            score -= 2
            plusScores += " "
            minusScores += "2"
        elif ch1 != ch2:
            score -= 1
            plusScores += " "
            minusScores += "1"
        else:
            score += 1
            plusScores += "1"
            minusScores += " "


    return score, plusScores, minusScores, strand1, strand2


# Utility function that generates a random DNA string of
# a random length drawn from the range [minlength, maxlength]
def generateRandomDNAStrand(minlength, maxlength):
	assert minlength > 0, \
	       "Minimum length passed to generateRandomDNAStrand" \
	       "must be a positive number" # these \'s allow mult-line statements
	assert maxlength >= minlength, \
	       "Maximum length passed to generateRandomDNAStrand must be at " \
	       "as large as the specified minimum length"
	strand = ""
	length = random.choice(xrange(minlength, maxlength + 1))
	bases = ['A', 'T', 'G', 'C']
	for i in xrange(0, length):
		strand += random.choice(bases)
	return strand

# Method that just prints out the supplied alignment score.
# This is more of a placeholder for what will ultimately
# print out not only the score but the alignment as well.
def printAlignment(score, out = sys.stdout):
	out.write("Optimal alignment score is " + str(score) + "\n")

# Unit test main in place to do little more than
# exercise the above algorithm.  As written, it
# generates two fairly short DNA strands and
# determines the optimal alignment score.
#
# As you change the implementation of findOptimalAlignment
# to use memoization, you should change the 8s to 40s and
# the 10s to 60s and still see everything execute very
# quickly.
def main():
        test(-4, '1   111  11 ', ' 122   12  2', 'CTTGAATG TA ', 'CA  AATCATAT')
        test(-5, '1 1     1 ', ' 1 11111 2', 'TAAAGGCTA ', 'TGACAAGCAC')
        test(-6, '1      1 ', ' 111111 2', 'AAAAAATT ', 'ACCCGTATA')
        test(-7, ' 11       ', '1  1111112', 'AGCCCAATAG', 'GGCTAGTCT ')
        test(-5, '  1     1', '11 11111 ', 'GTTATAATT', 'TATGCCCGT')
        test(-4, '    1 1  1', '1111 1 11 ', 'TAAAACTTAT', 'AGTCATTGTT')
        test(-8, '1        ', ' 11111112', 'TCCGCGGG ', 'TGTAAAACA')
        test(0, ' 1  1111 ', '1 11    2', 'TGTGCATT ', 'AGGTCATTG')
        test(0, '1 1 1 11 ', ' 1 1 1  2', 'TCACAATG ', 'TAAGACTGT')
        test(-4, '  1  11   ', '11 11  111', 'GCCAGTGGAC', 'ATCGCTGCGG')
        test(-8, '       1  ', '1111111 11', 'CTCATATTAT', 'GATCCGCTGA')

	while (True):
		sys.stdout.write("Generate random DNA strands? ")
		answer = sys.stdin.readline()
		if answer == "no\n": break
		strand1 = generateRandomDNAStrand(8, 10)
		strand2 = generateRandomDNAStrand(8, 10)
		sys.stdout.write("Aligning these two strands: " + strand1 + "\n")
		sys.stdout.write("                            " + strand2 + "\n")
		alignment = findOptimalAlignment(strand1, strand2)
		printAlignment(alignment)

if __name__ == "__main__":
  main()
