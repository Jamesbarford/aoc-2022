#!/usr/bin/env python3


nums = [
    0x582041,
    0x592041,
    0x5a2041,

    0x582042,
    0x592042,
    0x5a2042,

    0x582043,
    0x592043,
    0x5a2043,
]

def find_hash(modulo: int):
    slots = []
    ans = []
    for n in nums:
        h = n % modulo
        if h in slots:
            return False
        slots.append(h)
    for n in nums:
        key = str(hex(n))
        ans.append({key: n % modulo})

    print(ans)
    return True


def main():
    for i in range(100):
        modulo = i + 1
        if find_hash(modulo=modulo) == True:
            print(modulo)
            break

if __name__ == "__main__":
    main()
