define plist
  set var $n = $arg0
  while $n
    printf "%d ", $n->m_process->pid
    set var $n = $n->m_next
  end
  printf "\n"
end
