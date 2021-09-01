import git
import matplotlib.pyplot as plt

repo = git.Repo("/home/maritns3/core/ld/DuckBuBi")
commits = list(repo.iter_commits("main"))


fixme_array = [ ]
for m in commits:
    if "(FIXME " in m.message:
        txt = m.message.replace(")", ' ')
        nums = [int(s) for s in txt.split() if s.isdigit()]
        try:
            fixme_array.append(nums[0])
        except Exception as e:
            print(m.message)
            raise e

fixme_array = fixme_array[::-1]
print(fixme_array)

plt.plot(range(0, len(fixme_array)),fixme_array)
plt.title('fixme number after commit')
plt.xlabel('commit')
plt.ylabel('fixme')
plt.show()
