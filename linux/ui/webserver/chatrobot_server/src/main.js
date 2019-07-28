import Vue from 'vue'
import App from './App'
import { Layout,Slider,Menu,Icon,List,Avatar, Spin, Card} from 'ant-design-vue';
import infiniteScroll from 'vue-infinite-scroll'
import store from './vuex/store.js';

Vue.use(infiniteScroll)
Vue.use(Layout)
Vue.use(Slider)
Vue.use(Menu)
Vue.use(Icon)
Vue.use(List)
Vue.use(Avatar)
Vue.use(Spin)
Vue.use(Card)

Vue.config.productionTip = false

new Vue({
  store,
  render: h => h(App)
}).$mount("#app");