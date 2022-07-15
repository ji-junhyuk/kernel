let g:vim_wiki_set_path = expand('<sfile>:p:h')
let g:vimwiki_list = [
                        \{
                        \   'path': '~/diary/_diary',
                        \   'ext' : '.md',
                        \   'diary_rel_path': '.',
                        \},
                        \{
                        \   'path': '~/diary/_test',
                        \   'ext' : '.md',
                        \   'diary_rel_path': '.',
                        \},
                        \{
                        \   'path': '~/diary/_book',
                        \   'ext' : '.md',
                        \   'diary_rel_path': '.',
                        \},
                        \{
                        \   'path': '~/diary/_writing',
                        \   'ext' : '.md',
                        \   'diary_rel_path': '.',
                        \},
                        \{
                        \   'path': '~/diary/_status',
                        \   'ext' : '.md',
                        \   'diary_rel_path': '.',
                        \}
                        \]
let g:vimwiki_conceallevel = 0
let g:vimwiki_table_mappings = 0

autocmd BufNewFile 2022*.md call Generate_md()
function! Generate_md()
    let l:template = []
    call add(l:template, '---')
    call add(l:template, 'layout  : wiki')
    call add(l:template, 'title   : ')
    call add(l:template, 'summary : ')
    call add(l:template, 'date    : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
    call add(l:template, 'updated : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
    call add(l:template, 'tag     : ')
    call add(l:template, 'toc     : true')
    call add(l:template, 'public  : true')
    call add(l:template, 'parent  : ')
    call add(l:template, 'latex   : false')
    call add(l:template, '---')
    call add(l:template, '* TOC')
    call add(l:template, '{:toc}')
    call add(l:template, '')
    call add(l:template, '# ')
    call add(l:template, "- [ ] TDL")
    call add(l:template, "  - [ ] Routine")
    call add(l:template, "      - [ ] Algo")
    call add(l:template, "          - [ ] task")
    call add(l:template, "      - [ ] Vim")
    call add(l:template, "          - [ ] task")
    call add(l:template, "      - [ ] Book")
    call add(l:template, "          - [ ] task")
    call add(l:template, "      - [ ] Exam")
    call add(l:template, "          - [ ] task")
    call add(l:template, "  - [ ] Main")
    call add(l:template, "      - [ ] 진도")
    call add(l:template, "      - [ ] ejae, 8solve")
    call add(l:template, "      - [ ] ejae, 8evaluate")
    call setline(1, l:template)
    execute 'normal! G'
    execute 'normal! $'

    echom 'new wiki page has created'
endfunction

" 자주 사용하는 vimwiki 명령어에 단축키를 취향대로 매핑해둔다
command! WikiIndex :VimwikiIndex
nmap <LocalLeader>ww <Plug>VimwikiIndex
nmap <LocalLeader>wi <Plug>VimwikiDiaryIndex
nmap <LocalLeader>w<LocalLeader>w <Plug>VimwikiMakeDiaryNote
nmap <LocalLeader>wt :VimwikiTable<CR>

" F4 키를 누르면 커서가 놓인 단어를 위키에서 검색한다.
nnoremap <F4> :execute "VWS /" . expand("<cword>") . "/" <Bar> :lopen<CR>

" Shift F4 키를 누르면 현재 문서를 링크한 모든 문서를 검색한다
nnoremap <S-F4> :execute "VWB" <Bar> :lopen<CR>

" If buffer modified, update any 'Last modified: ' in the first 20 lines.
" 'Last modified: ' can have up to 10 characters before (they are retained).
" Restores cursor and window position using save_cursor variable.
function! LastModified()
    if g:md_modify_disabled
        return
    endif

    if (&filetype != "vimwiki")
        return
    endif

    if &modified
        " echo('markdown updated time modified')
        let save_cursor = getpos(".")
        let n = min([10, line("$")])

        exe 'keepjumps 1,' . n . 's#^\(.\{,10}updated\s*: \).*#\1' .
                    \ strftime('%Y-%m-%d %H:%M:%S +0900') . '#e'
        call histdel('search', -1)
        call setpos('.', save_cursor)
    endif
endfunction
function! NewTemplate()

    let l:wiki_directory = v:false

    for wiki in g:vimwiki_list
        if expand('%:p:h') =~ expand(wiki.path)
            let l:wiki_directory = v:true
            break
        endif
    endfor

    if !l:wiki_directory
        return
    endif

    if line("$") > 1
        return
    endif

    let l:template = []
    call add(l:template, '---')
    call add(l:template, 'layout  : wiki')
    call add(l:template, 'title   : ')
    call add(l:template, 'summary : ')
    call add(l:template, 'date    : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
    call add(l:template, 'updated : ' . strftime('%Y-%m-%d %H:%M:%S +0900'))
    call add(l:template, 'tag     : ')
    call add(l:template, 'toc     : true')
    call add(l:template, 'public  : true')
    call add(l:template, 'parent  : ')
    call add(l:template, 'latex   : false')
    call add(l:template, '---')
    call add(l:template, '* TOC')
    call add(l:template, '{:toc}')
    call add(l:template, '')
    call add(l:template, '# ')
    call setline(1, l:template)
    execute 'normal! G'
    execute 'normal! $'

    echom 'new wiki page has created'
endfunction
augroup vimwikiauto
    autocmd BufWritePre *.md keepjumps call LastModified()
    autocmd BufRead,BufNewFile *.md call NewTemplate()
    autocmd FileType vimwiki inoremap <S-Right> <C-r>=vimwiki#tbl#kbd_tab()<CR>
    autocmd FileType vimwiki inoremap <S-Left> <Left><C-r>=vimwiki#tbl#kbd_shift_tab()<CR>
augroup END

function! UpdateBookProgress()
    let l:cmd = g:vim_wiki_set_path . "/bookProgressUpdate.sh " . expand('%:p')
    call system(l:cmd)
    edit
endfunction

augroup todoauto
    autocmd BufWritePost /mnt/c/Users/junto/OneDrive/_book/index.md call UpdateBookProgress()
augroup END

let g:md_modify_disabled = 0
