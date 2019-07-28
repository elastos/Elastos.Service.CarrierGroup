import Vue from 'vue'
import Vuex from 'vuex'
Vue.use(Vuex)

/*1.state在vuex中用于存储数据*/
var state={
    focusInfo:""
}
var mutations={
    setFocusInfo(info){
        state.focusInfo = info;
    },
}

const store = new Vuex.Store({
    state,
    mutations
})

export default store;