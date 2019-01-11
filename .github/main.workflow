workflow "New workflow" {
  on = "push"
  resolves = ["Docker Tag"]
}

action "Test within WinCE" {
  uses = "navit-gps/Dockerfiles/wince@master"
  args = "ls"
}

action "GitHub Action for Docker" {
  uses = "navit-gps/Dockerfiles/wince@master"
  args = "touch 123"
  needs = ["Test within WinCE"]
}

action "Docker Tag" {
  uses = "navit-gps/Dockerfiles/wince@master"
  args = "ls"
  
  needs = ["GitHub Action for Docker"]
}
