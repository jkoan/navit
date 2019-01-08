workflow "New workflow" {
  on = "push"
  resolves = ["GitHub Action for Docker"]
}

action "Test within WinCE" {
  uses = "navit-gps/Dockerfiles/wince@master"
  args = "echo blsa"
}
