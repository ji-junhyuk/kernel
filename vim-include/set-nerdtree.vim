" Vim 실행시 NERDTree를 시작하고 커서를 다른 창에 다시 놓습니다.
" autocmd  VimEnter  * NERDTree | wincmd  p

nnoremap <LocalLeader>e :NERDTreeToggle<CR>
nnoremap <LocalLeader><LocalLeader>e :NERDTreeFind<CR>
let NERDTreeShowHidden = 1 "Show hidden files in NerdTree
