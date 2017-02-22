@echo OFF
for %%G in (test\test*.txt) do (
    windowreader.exe < %%G > %%G.answer
    fc %%G.answer %%G.result
)
