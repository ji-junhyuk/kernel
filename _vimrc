set tabstop=4
set shiftwidth=4
set cindent
set number
syntax on
set backspace=2
set backspace=indent,eol,start
set showmatch
set ruler
set history=1000
set vb t_vb= " 경고음 소리를 깜빡임으로 대체

nnoremap <leader>t :tabnew
nnoremap <leader>q :bn<CR>
nnoremap <leader>w :bp<CR>

map <buffer> <F2> :w<CR>
map <buffer> <F3> :!norminette -R CheckForbiddenSourceHeader *.*<CR>
map <buffer> <F4> :!g++ -Wall -Werror -Wextra *.c && %<<CR>
map <buffer> <F5> :!cl % && %< <CR>
map <buffer> <F6> :!"C:\Program Files (x86)\Windows Kits\10\Debuggers\x86\cdb.exe" %< <CR>
