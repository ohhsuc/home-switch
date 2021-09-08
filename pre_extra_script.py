Import("env")

# access to global construction environment
print(env)

env.Execute("cd lib/VictoriaWeb && npm run build")
