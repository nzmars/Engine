# git 관려 전략 검토

- ORE (Open Source Risk Engine)은 https://github.com/opensourcerisk/engine 에 있는 코드를 원본으로 하여 나의 repository에 fork하여 사용하고 있다.
- 나의 repository는 original ORE를 받아 필요한 부분을 수정하여 사용하고 있으며, build를 쉽게 하기 위해 scripts를 추가하여 사용하고 있다.
- 원천 코드 ORE 기반를 바탕으로 following하면서 내가 customize한 부분을 유지하고 싶다.
- 이를 위해 git branch 전략 및 git merge 전략을 검토하고자 한다.
- 어떤 branch 전략을 사용해야 하는지, merge 전략은 어떻게 해야 하는지에 대한 검토가 필요하며 이를 위해 문서화(markdown)하여 앞으로 어떻게 해야할지 검토해줘. 
- 주기적으로 original ORE repository를 반영하면서 나의 repository를 유지보수할 수 있는 전략을 세워야 한다.
