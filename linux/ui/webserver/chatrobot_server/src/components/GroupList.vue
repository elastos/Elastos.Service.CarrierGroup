<template>
<div
  class="demo-infinite-container"
  v-infinite-scroll="handleInfiniteOnLoad"
  :infinite-scroll-disabled="busy"
  :infinite-scroll-distance="10"
>
  <a-list
    :dataSource="data"
  >
    <a-list-item slot="renderItem" slot-scope="item">
      <a-list-item-meta :description="item.address">
        <a slot="title" >{{item.id }}:{{item.nickname?item.nickname:"未命名"}}</a>
        <a-avatar slot="avatar" src="https://zos.alipayobjects.com/rmsportal/ODTLcjxAfvqbxHnVXCYX.png" />
      </a-list-item-meta>
      <div>{{item.members}}</div>
    </a-list-item>
    <div v-if="loading && !busy" class="demo-loading-container">
      <a-spin />
    </div>
  </a-list>
</div>
</template>
<script>
import reqwest from 'reqwest'
import infiniteScroll from 'vue-infinite-scroll'
const fakeDataUrl = 'http://127.0.0.1:8384/groups'
export default {
  directives: { infiniteScroll },
  data () {
    return {
      data:[],
      loading: false,
      curAddressIdx:0,
      busy: false,
    }
  },
  beforeMount () {
    this.fetchData((res) => {
       this.data = JSON.parse(res.data)
       let cur_info = null
       if (this.curAddressIdx >= this.data.length){
         this.curAddressIdx = this.data.length - 1;
         cur_info = this.data[this.curAddressIdx];
       }
       this.$store.commit('setFocusInfo', cur_info)
       this.loading = false
    })
  },
  methods: {
    fetchData (callback) {
      reqwest({
        url: fakeDataUrl,
        type: 'json',
        method: 'get',
        contentType: 'application/json',
        success: (res) => {
          callback(res)
        },
      })
    },
    handleInfiniteOnLoad  () {
      const data = this.data
      this.loading = true
      if (data.length > 14) {
        this.$message.warning('Infinite List loaded all')
        this.busy = true
        this.loading = false
        return
      }
      this.fetchData((res) => {
         this.data = JSON.parse(res.data)
        let cur_info = null
        if (this.curAddressIdx >= this.data.length){
         this.curAddressIdx = this.data.length - 1;
         cur_info = this.data[this.curAddressIdx];
        }
        this.$store.commit('setFocusInfo', cur_info)
        this.loading = false
      })
    },
  },
}
</script>
<style>
.demo-infinite-container {
  border: 1px solid #e8e8e8;
  border-radius: 4px;
  overflow: auto;
  padding: 8px 24px;
  width:800px;
  height: 600px;
}
.demo-loading-container {
  position: absolute;
  bottom: 40px;
  width: 100%;
}
</style>
