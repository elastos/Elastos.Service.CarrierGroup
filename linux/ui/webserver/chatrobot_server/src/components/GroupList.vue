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
      <a-list-item slot="renderItem" slot-scope="item" key="item.id">
          <a-list-item-meta
                  :description="item.address"
          >
              <a slot="title" >{{item.id }}:{{item.nickname?item.nickname:"未命名"}}</a>
              <a-avatar slot="avatar" src="https://zos.alipayobjects.com/rmsportal/ODTLcjxAfvqbxHnVXCYX.png" />

          </a-list-item-meta>
          <a-card>成员数：{{item.members}}
              <qrcode :value="item.address" :options="{ width: 200 }"></qrcode>
          </a-card>


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
const fakeDataUrl = window.location.href.replace("8080","8081")+'/groups'
export default {
  directives: { infiniteScroll },
  data () {
    return {
      data:[],
      loading: false,
      curId:0,
      busy: false,
    }
  },
  beforeMount () {
    this.fetchData((res) => {
       this.data = JSON.parse(res.data)
       this.updateCurInfo();
       this.loading = false
    })
  },
  methods: {
    updateCurInfo() {
     let cur_info = null;
        for(let i=0; i<this.data.length;i++) {
            let item = this.data[i];
            if (item.id == this.curId) {
                cur_info = item;
                break;
            }
       }
       if (cur_info != null) {
        this.$store.commit('focusinfo', cur_info)
       }
    },
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
    updateQrcode(id){

     this.curId = id;
     this.updateCurInfo();
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
      window.console.log("handleInfiniteOnLoad in");
        this.fetchData((res) => {
           this.data = JSON.parse(res.data)
           this.updateCurInfo();
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
  height: 600px;
}
.demo-loading-container {
  position: absolute;
  bottom: 40px;
  width: 100%;
}
</style>


