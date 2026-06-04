from metrices import lsd, mfcc, sc

# UNC:str = "signals/uncompressed.wav"
# COMP:str = "signals/compressed.wav"

UNC:str = "signals/1.wav"
COMP:str = "signals/2.wav"

if __name__ == "__main__":
    print("LSD:", lsd(UNC, COMP, COMP))
    print("SC:", sc(UNC, COMP, COMP))
    print("MFCC:", mfcc(UNC, COMP, COMP))
